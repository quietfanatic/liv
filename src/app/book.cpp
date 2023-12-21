#include "book.h"

#include <filesystem>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include "../dirt/geo/scalar.h"
#include "../dirt/glow/gl.h"
#include "../dirt/uni/time.h"
#include "app.h"
#include "files.h"
#include "layout.h"
#include "memory.h"
#include "page.h"

namespace app {

///// Contents

Book::Book (
    App& app,
    Slice<AnyString> page_filenames,
    const AnyString& book_filename,
    const AnyString& start_filename
) :
    settings(app.settings),
    memory(app.memory),
    block(book_filename, page_filenames),
    window_background(
        settings->get(&WindowSettings::window_background)
    ),
    window(
        "Little Image Viewer",
        settings->get(&WindowSettings::size)
    )
{
     // Figure out where to start and initialize view params from memory if
     // applicable.
    const MemoryOfBook* mem = null;
    if (book_filename) {
        for (auto& m : memory->books) {
            if (m.book_filename == book_filename) {
                mem = &m;
                break;
            }
        }
    }
    if (mem) {
        layout_params = mem->layout_params;
        page_params = mem->page_params;
    }
    else {
        layout_params = LayoutParams(settings);
        page_params = PageParams(settings);
    }
     // Find start page
    int32 offset = 0;
    const AnyString& start =
        start_filename ? start_filename :
        mem ? mem->current_filename : "";
    if (start) {
        for (usize i = 0; i < page_filenames.size(); i++) {
            if (page_filenames[i] == start) {
                offset = i;
                break;
            }
        }
    }
    viewing_pages = {
        offset, offset + settings->get(&LayoutSettings::spread_count)
    };
     // Set up window
    SDL_SetWindowResizable(window, SDL_TRUE);
    expect(!SDL_GL_SetSwapInterval(1));
    if (settings->get(&WindowSettings::fullscreen)) {
        set_fullscreen(true);
    }
    glow::init();
    if (!app.hidden) SDL_ShowWindow(window);
}
Book::~Book () { }

static void update_memory (Book& self) {
    if (self.block.book_filename) {
        MemoryOfBook mem;
        mem.book_filename = self.block.book_filename;
        mem.current_offset = self.viewing_pages.l;
        if (auto page = self.block.get(self.viewing_pages.l)) {
            mem.current_filename = page->filename;
        }
        else mem.current_filename = "";
        mem.layout_params = self.layout_params;
        mem.page_params = self.page_params;
        mem.updated_at = uni::now();
        for (auto& m : self.memory->books) {
            if (m.book_filename == self.block.book_filename) {
                m = move(mem);
                goto done;
            }
        }
        self.memory->books.emplace_back(move(mem));
        done:
        self.memory->need_write = true;
    }
}

///// Controls

void Book::set_window_background (Fill bg) {
    window_background = bg;
    need_draw = true;
}

void Book::set_page_offset (int32 off) {
    if (!block.count()) return;
     // Clamp such that there is at least one visible page in the range
    int32 l = clamp(off - 1, 1 - size(viewing_pages), block.count() - 1);
    viewing_pages = {l, l + size(viewing_pages)};
    expect(size(visible_pages()) >= 1);
    if (settings->get(&LayoutSettings::reset_zoom_on_page_turn)) {
        layout_params.manual_zoom = GNAN;
        layout_params.manual_offset = GNAN;
    }
    spread = {};
    layout = {};
    need_draw = true;
    update_memory(*this);
}

void Book::set_spread_count (int32 count) {
     // TODO: clamp viewing_pages.l too
    viewing_pages.r = viewing_pages.l + clamp(count, 1, 2048);
    spread = {};
    layout = {};
    need_draw = true;
    update_memory(*this);
}

void Book::set_spread_direction (SpreadDirection dir) {
    layout_params.spread_direction = dir;
    spread = {};
    layout = {};
    need_draw = true;
    update_memory(*this);
}

void Book::set_align (Vec small, Vec large) {
    if (defined(small.x)) layout_params.small_align.x = small.x;
    if (defined(small.y)) layout_params.small_align.y = small.y;
    if (defined(large.x)) layout_params.large_align.x = large.x;
    if (defined(large.y)) layout_params.large_align.y = large.y;
    layout_params.manual_offset = GNAN;
    spread = {};
    layout = {};
    need_draw = true;
    update_memory(*this);
}

void Book::set_auto_zoom_mode (AutoZoomMode mode) {
    layout_params.auto_zoom_mode = mode;
    layout_params.manual_zoom = GNAN;
    layout_params.manual_offset = GNAN;
    layout = {};
    need_draw = true;
    update_memory(*this);
}

void Book::set_interpolation_mode (InterpolationMode mode) {
    page_params.interpolation_mode = mode;
    need_draw = true;
    update_memory(*this);
}

void Book::drag (Vec amount) {
    if (!defined(layout_params.manual_offset)) {
        auto& layout = get_layout();
        layout_params.manual_offset = layout.offset;
        layout_params.manual_zoom = layout.zoom;
    }
    layout_params.manual_offset += amount;
    layout = {};
    need_draw = true;
    update_memory(*this);
}

void Book::zoom_multiply (float factor) {
     // Need spread to clamp the zoom
    auto& spread = get_spread();
     // Actually we also need the layout to multiply the zoom
    auto& layout = get_layout();
     // Set manual zoom
    layout_params.manual_zoom = spread.clamp_zoom(settings, layout.zoom * factor);
    if (defined(layout_params.manual_offset)) {
         // Hacky way to zoom from center
         // TODO: zoom to preserve current alignment
        layout_params.manual_offset +=
            spread.size * (layout.zoom - layout_params.manual_zoom) / 2;
    }
    this->layout = {};
    need_draw = true;
    update_memory(*this);
}

void Book::reset_layout () {
    layout_params = LayoutParams(settings);
    spread = {};
    layout = {};
    need_draw = true;
    update_memory(*this);
}

bool Book::is_fullscreen () const {
    auto flags = glow::require_sdl(SDL_GetWindowFlags(window));
    return flags & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN);
}

void Book::set_fullscreen (bool fs) {
     // This will trigger a window_size_changed, so no need to clear layout or
     // set need_draw
    glow::require_sdl(!SDL_SetWindowFullscreen(
        window,
        fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
    ));
}

bool Book::is_minimized () const {
    auto flags = glow::require_sdl(SDL_GetWindowFlags(window));
    return flags & SDL_WINDOW_MINIMIZED;
}

///// Internal stuff

const Spread& Book::get_spread () {
     // Not sure this is the best place to do this.
    if (!spread) spread.emplace(block, viewing_pages, layout_params);
    return *spread;
}

const Layout& Book::get_layout () {
    if (!layout) {
        layout.emplace(
            settings, get_spread(),
            layout_params, get_window_size()
        );
    }
    return *layout;
}

bool Book::draw_if_needed () {
    if (!need_draw) return false;
    need_draw = false;
    auto& spread = get_spread();
    auto& layout = get_layout();
     // TODO: Currently we have a different context for each window, would it
     // be better to share a context between all windows?
    SDL_GL_MakeCurrent(window, window.gl_context);
     // Draw background
    glClearColor(
        window_background.r / 255.0,
        window_background.g / 255.0,
        window_background.b / 255.0,
        window_background.a / 255.0 // Alpha is probably ignored
    );
    glClear(GL_COLOR_BUFFER_BIT);
     // Draw spread
    Vec window_size = get_window_size();
    for (auto& spread_page : spread.pages) {
        spread_page.page->last_viewed_at = uni::now();
        Rect spread_rect = Rect(
            spread_page.offset,
            spread_page.offset + spread_page.page->size
        );
        Rect window_rect = spread_rect * layout.zoom + layout.offset;
         // Convert to OpenGL coords (-1,-1)..(+1,+1)
        Rect screen_rect = window_rect / window_size * float(2) - Vec(1, 1);
         // Draw
        spread_page.page->draw(page_params, layout.zoom, screen_rect);
    }
     // Generate title
    AnyString title;
    IRange visible = visible_pages();
    if (block.count() == 0) {
        title = "Little Image Viewer (nothing loaded)";
    }
    else if (empty(visible)) {
        title = "Little Image Viewer (no pages visible)";
    }
    else {
        if (block.count() > 1) {
            if (size(visible) == 1) {
                title = cat('[', visible.l+1);
            }
            else if (size(visible) == 2) {
                title = cat('[', visible.l+1, ',', visible.r+1 - 1);
            }
            else {
                title = cat('[', visible.l+1, '-', visible.r+1 - 1);
            }
            title = cat(move(title), '/', block.count(), "] ");
        }
         // TODO: Merge filenames
        title = cat(move(title), block.get(visible.l)->filename);
         // In general, direct comparisons of floats are not good, but we do
         // slight snapping of our zoom to half-integers, so this is fine.
        if (layout.zoom != 1) {
            title = cat(move(title), " (", geo::round(layout.zoom * 100), "%)");
        }
    }
    SDL_SetWindowTitle(window, title.c_str());
     // vsync
    SDL_GL_SwapWindow(window);
    return true;
}

bool Book::idle_processing () {
    return block.idle_processing(this, settings);
}

IVec Book::get_window_size () const {
    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    require(w > 0 && h > 0);
    return {w, h};
}

void Book::window_size_changed (IVec size) {
    require(size.x > 0 && size.y > 0);
    glViewport(0, 0, size.x, size.y);
    layout = {};
    need_draw = true;
}

} using namespace app;

#ifndef TAP_DISABLE_TESTS
#include <SDL2/SDL.h>
#include "../dirt/ayu/resources/resource.h"
#include "../dirt/glow/image.h"
#include "../dirt/tap/tap.h"

static tap::TestSet tests ("app/book", []{
    using namespace tap;

    char* base = glow::require_sdl(SDL_GetBasePath());
    auto exe_folder = UniqueString(base);
    SDL_free(base);

    IVec size = {120, 120};

    App app;
    //app.hidden = true;
    app.settings->WindowSettings::size = size;
    Book book (app, {
        cat(exe_folder, "/res/dirt/glow/test/image.png"sv),
        cat(exe_folder, "/res/dirt/glow/test/image2.png"sv)
    });

    book.draw_if_needed();
    glow::Image img (size);
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.get_page_offset(), 1, "Initial page is 1");
    is(img[{60, 60}], glow::RGBA8(0x2674dbff), "First page is correct");

    book.next();
    book.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.get_page_offset(), 2, "Next page is 2");
    is(img[{60, 60}], glow::RGBA8(0x45942eff), "Second page is correct");

    book.next();
    is(book.get_page_offset(), 2, "Can't go past last page");
    book.seek(10000);
    is(book.get_page_offset(), 2, "Can't seek past last page");

    book.prev();
    book.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.get_page_offset(), 1, "Go back to page 1");
    is(img[{60, 60}], glow::RGBA8(0x2674dbff), "Going back to first page works");

    book.prev();
    book.draw_if_needed();
    is(book.get_page_offset(), 1, "Can't go before page 1");

    is(img[{0, 60}], glow::RGBA8(0x2674dbff), "Default to auto_zoom_mode = fit");
    book.set_auto_zoom_mode(ORIGINAL);
    book.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{0, 60}], glow::RGBA8(0x000000ff), "auto_zoom_mode = original");

    book.set_auto_zoom_mode(FIT);
    book.set_spread_count(2);
    book.set_page_offset(1);
    is(book.viewing_pages, IRange{0, 2}, "Viewing two pages");
    is(book.visible_pages(), IRange{0, 2}, "Two visible pages");
    book.draw_if_needed();
    is(book.spread->pages.size(), usize(2), "Spread has two pages");
    is(book.spread->pages[1].offset.x, 7, "Spread second page has correct offset");
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{20, 60}], glow::RGBA8(0x2674dbff), "Left side of spread is correct");
    is(img[{100, 60}], glow::RGBA8(0x45942eff), "Right side of spread is correct");
    is(img[{20, 30}], glow::RGBA8(0x000000ff), "Spread doesn't fill too much area");

    book.set_spread_direction(LEFT);
    book.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{20, 60}], glow::RGBA8(0x45942eff), "spread direction left (left)");
    is(img[{100, 60}], glow::RGBA8(0x2674dbff), "spread direction left (right)");
    is(img[{20, 30}], glow::RGBA8(0x000000ff), "Spread doesn't fill too much area");

    book.next();
    is(book.viewing_pages, IRange{1, 3}, "viewing_pages can go off the end");
    is(book.visible_pages(), IRange{1, 2}, "visible_pages cannot go off the end");

    done_testing();
});

#endif
