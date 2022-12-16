#include "book.h"

#include <filesystem>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include "../base/geo/scalar.h"
#include "../base/glow/gl.h"
#include "../base/uni/time.h"
#include "app.h"
#include "files.h"
#include "layout.h"
#include "page.h"

namespace app {

///// Private helpers

static void update_title (Book& self) {
    String title;
    if (self.pages.size() == 0) {
        title = "Little Image Viewer (nothing loaded)"s;
    }
    else {
        if (self.pages.size() > 1) {
            isize first = self.first_visible_page();
            isize last = self.last_visible_page();
            if (first == last) {
                title = ayu::cat('[', first);
            }
            else if (first + 1 == last) {
                title = ayu::cat('[', first, ',', last);
            }
            else {
                title = ayu::cat('[', first, '-', last);
            }
            title = ayu::cat(title, '/', self.pages.size(), "] ");
        }
         // TODO: Merge filenames
        title += self.get_page(self.first_visible_page())->filename;
         // In general, direct comparisons of floats are not good, but we do
         // slight snapping of our floats to half-integers, so this is fine.
        if (self.layout && self.layout->zoom != 1) {
            title = ayu::cat(title,
                " (", geo::round(self.layout->zoom * 100), "%)"
            );
        }
    }
    SDL_SetWindowTitle(self.window, title.c_str());
}

static void load_page (Book& self, Page* page) {
    if (page && !page->texture) {
        page->load();
        self.estimated_page_memory += page->estimated_memory;
    }
}

static void unload_page (Book& self, Page* page) {
    if (page && page->texture) {
        page->unload();
        self.estimated_page_memory -= page->estimated_memory;
        DA(self.estimated_page_memory >= 0);
    }
}

static const Spread& get_spread (Book& self) {
     // Not sure this is the best place to do this.
    for (isize no = self.first_visible_page();
         no <= self.last_visible_page();
         no++
    ) {
        load_page(self, self.get_page(no));
    }
    if (!self.spread) self.spread.emplace(self, self.layout_params);
    return *self.spread;
}

static const Layout& get_layout (Book& self) {
    if (!self.layout) {
        self.layout.emplace(
            self.settings, get_spread(self),
            self.layout_params, self.get_window_size()
        );
    }
    return *self.layout;
}

///// Contents

Book::Book (App& app, FilesToOpen&& to_open) :
    settings(app.settings),
    folder(std::move(to_open.folder)),
    page_offset(to_open.start_index + 1),
    window_background(
        settings->get(&WindowSettings::window_background)
    ),
    layout_params(settings),
    page_params(settings),
    window(
        "Little Image Viewer",
        settings->get(&WindowSettings::size)
    )
{
    SDL_SetWindowResizable(window, SDL_TRUE);
    DA(!SDL_GL_SetSwapInterval(1));

    if (settings->get(&WindowSettings::fullscreen)) {
        set_fullscreen(true);
    }
    glow::init();
    pages.reserve(to_open.files.size());
    for (auto& filename : to_open.files) {
        pages.emplace_back(std::make_unique<Page>(std::move(filename)));
    }
    if (!app.hidden) SDL_ShowWindow(window);
}
Book::~Book () { }

isize Book::clamp_page_offset (isize off) const {
    if (!pages.empty()) return clamp(
        off, 1 - (spread_pages-1), isize(pages.size()) + (spread_pages-1)
    );
    else return 1;
}

Page* Book::get_page (isize no) const {
    if (!pages.size() || no < 1 || no > isize(pages.size())) return null;
    else return &*pages[no-1];
}

///// Window parameters

void Book::set_window_background (Fill bg) {
    window_background = bg;
    need_draw = true;
}

///// Controls

void Book::set_page_offset (isize off) {
    page_offset = clamp_page_offset(off);
    if (settings->get(&LayoutSettings::reset_zoom_on_page_turn)) {
        layout_params.manual_zoom = NAN;
        layout_params.manual_offset = NAN;
    }
    spread = {};
    layout = {};
    need_draw = true;
}

void Book::set_spread_pages (isize count) {
    if (count < 1) count = 1;
    else {
        isize max = settings->get(&LayoutSettings::max_spread_pages);
        if (max < 1) max = 1;
        if (count > max) count = max;
    }
    spread_pages = count;
    spread = {};
    layout = {};
    need_draw = true;
}

void Book::set_spread_direction (SpreadDirection dir) {
    layout_params.spread_direction = dir;
    spread = {};
    layout = {};
    need_draw = true;
}

void Book::set_align (Vec small, Vec large) {
    if (defined(small.x)) layout_params.small_align.x = small.x;
    if (defined(small.y)) layout_params.small_align.y = small.y;
    if (defined(large.x)) layout_params.large_align.x = large.x;
    if (defined(large.y)) layout_params.large_align.y = large.y;
    layout_params.manual_offset = NAN;
    spread = {};
    layout = {};
    need_draw = true;
}

void Book::set_auto_zoom_mode (AutoZoomMode mode) {
    layout_params.auto_zoom_mode = mode;
    layout_params.manual_zoom = NAN;
    layout_params.manual_offset = NAN;
    layout = {};
    need_draw = true;
}

void Book::set_interpolation_mode (InterpolationMode mode) {
    page_params.interpolation_mode = mode;
    need_draw = true;
}

void Book::drag (Vec amount) {
    if (defined(layout_params.manual_offset)) {
        layout_params.manual_offset += amount;
    }
    else {
        layout_params.manual_offset = amount;
    }
    layout = {};
    need_draw = true;
}

void Book::zoom_multiply (float factor) {
     // Need spread to clamp the zoom
    auto& spread = get_spread(*this);
     // Actually we also need the layout to multiply the zoom
    auto& layout = get_layout(*this);
     // Set manual zoom
    layout_params.manual_zoom = spread.clamp_zoom(settings, layout.zoom * factor);
    if (defined(layout_params.manual_offset)) {
         // Hacky way to zoom from center
         // TODO: zoom to preserve current alignment
        layout_params.manual_offset +=
            spread.size * (layout.zoom - layout_params.manual_zoom) / 2;
    }
}

void Book::reset_layout () {
    layout_params = LayoutParams(settings);
    spread = {};
    layout = {};
    need_draw = true;
}

bool Book::is_fullscreen () const {
    auto flags = AS(SDL_GetWindowFlags(window));
    return flags & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN);
}

void Book::set_fullscreen (bool fs) {
     // This will trigger a window_size_changed, so no need to clear layout or
     // set need_draw
    AS(!SDL_SetWindowFullscreen(
        window,
        fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
    ));
}

///// Internal stuff

bool Book::draw_if_needed () {
    if (!need_draw) return false;
    need_draw = false;
    auto& spread = get_spread(*this);
    auto& layout = get_layout(*this);
     // TODO: Currently we have a different context for each window, would it
     // be better to share a context between all windows?
    SDL_GL_MakeCurrent(window, window.gl_context);
    glClearColor(
        window_background.r / 255.0,
        window_background.g / 255.0,
        window_background.b / 255.0,
        window_background.a / 255.0 // Alpha is probably ignored
    );
    glClear(GL_COLOR_BUFFER_BIT);
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
    update_title(*this);
    SDL_GL_SwapWindow(window);
    return true;
}

bool Book::idle_processing () {
    int32 preload_ahead = settings->get(&MemorySettings::preload_ahead);
    int32 preload_behind = settings->get(&MemorySettings::preload_behind);
    int32 page_cache_mb = settings->get(&MemorySettings::page_cache_mb);

    isize preload_first = max(page_offset - preload_behind, 1);
    isize preload_last = min(
        page_offset + spread_pages - 1 + preload_ahead,
        isize(pages.size())
    );
     // Preload pages forwards
    for (isize no = page_offset; no >= preload_first; no--) {
        if (Page* page = get_page(no)) {
            if (!page->texture && !page->load_failed) {
                load_page(*this, page);
                return true;
            }
        }
    }
     // Preload pages backwards
    for (isize no = page_offset + spread_pages - 1; no <= preload_last; no++) {
        if (Page* page = get_page(no)) {
            if (!page->texture && !page->load_failed) {
                load_page(*this, page);
                return true;
            }
        }
    }
     // Unload pages if we're above the memory limit
    isize limit = page_cache_mb * (1024*1024);
    if (estimated_page_memory > limit) {
        double oldest_viewed_at = INF;
        Page* oldest_page = null;
        for (isize no = 1; no <= isize(pages.size()); no++) {
             // Don't unload images in the preload region
            if (no >= preload_first && no <= preload_last) {
                continue;
            }
            Page* page = get_page(no);
            if (!page->texture) continue;
            if (page->last_viewed_at < oldest_viewed_at) {
                oldest_viewed_at = page->last_viewed_at;
                oldest_page = page;
            }
        }
        if (oldest_page) {
            unload_page(*this, oldest_page);
        }
    }
     // Didn't do anything
    return false;
}

IVec Book::get_window_size () const {
    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    AA(w > 0 && h > 0);
    return {w, h};
}

void Book::window_size_changed (IVec size) {
    AA(size.x > 0 && size.y > 0);
    glViewport(0, 0, size.x, size.y);
    layout = {};
    need_draw = true;
}

} using namespace app;

#ifndef TAP_DISABLE_TESTS
#include <SDL2/SDL.h>
#include "../base/ayu/resource.h"
#include "../base/glow/image.h"
#include "../base/tap/tap.h"

static tap::TestSet tests ("app/book", []{
    using namespace tap;

    char* base = AS(SDL_GetBasePath());
    String exe_folder = base;
    SDL_free(base);

    IVec size = {120, 120};

    App app;
    app.hidden = true;
    app.settings->WindowSettings::size = size;
    Book book (app, FilesToOpen{{
        ayu::cat(exe_folder, "/res/base/glow/test/image.png"sv),
        ayu::cat(exe_folder, "/res/base/glow/test/image2.png"sv)
    }});

    book.draw_if_needed();
    glow::Image img (size);
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.page_offset, 1, "Initial page is 1");
    is(img[{60, 60}], glow::RGBA8(0x2674dbff), "First page is correct");

    book.next();
    book.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.page_offset, 2, "Next page is 2");
    is(img[{60, 60}], glow::RGBA8(0x45942eff), "Second page is correct");

    book.next();
    book.draw_if_needed();
    is(book.page_offset, 2, "Can't go past last page");

    book.prev();
    book.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.page_offset, 1, "Go back to page 1");
    is(img[{60, 60}], glow::RGBA8(0x2674dbff), "Going back to first page works");

    book.prev();
    book.draw_if_needed();
    is(book.page_offset, 1, "Can't go before page 1");

    is(img[{0, 60}], glow::RGBA8(0x2674dbff), "Default to auto_zoom_mode = fit");
    book.set_auto_zoom_mode(ORIGINAL);
    book.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{0, 60}], glow::RGBA8(0x000000ff), "auto_zoom_mode = original");

    book.set_auto_zoom_mode(FIT);
    book.set_spread_pages(2);
    book.set_page_offset(1);
    is(book.first_visible_page(), 1, "first_visible_page()");
    is(book.last_visible_page(), 2, "last_visible_page()");
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

    done_testing();
});

#endif
