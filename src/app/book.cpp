#include "book.h"

#include <filesystem>
#include <SDL2/SDL_video.h>
#include "../base/geo/scalar.h"
#include "../base/glow/gl.h"
#include "../base/uni/time.h"
#include "app.h"
#include "page.h"
#include "settings.h"

using namespace geo;
namespace fs = std::filesystem;

namespace app {

static void update_title (Book& self) {
    String title;
    if (self.pages.size() == 0) {
        title = "Little Image Viewer (no pages loaded)"s;
    }
    else {
        if (self.pages.size() > 1) {
            title = "[" + std::to_string(self.current_page_no)
                  + "/" + std::to_string(self.pages.size()) + "] ";
        }
        title += self.get_page(self.current_page_no)->filename;
    }
    self.window.title = std::move(title);
    self.window.update();
}

Book::Book (App& app, Str folder) :
    app(app),
    folder(folder)
{
    AA(false);
}
Book::Book (App& app, const std::vector<String>& filenames) :
    app(app),
    fit_mode(app.settings->page.fit_mode),
    interpolation_mode(app.settings->page.interpolation_mode),
    window{
        .title = "Little Image Viewer",
        .size = app.settings->window.size,
        .resizable = true,
        .hidden = app.hidden
    }
{
    window.open();
    if (app.settings->window.fullscreen) {
        set_fullscreen(true);
    }
    glow::init();
    pages.reserve(filenames.size());
    for (auto& filename : filenames) {
        pages.emplace_back(std::make_unique<Page>(filename));
    }
    update_title(*this);
}
Book::~Book () { }

bool Book::valid_page_no (isize no) {
    return no >= 1 && usize(no) <= pages.size();
}

Page* Book::get_page (isize no) {
    if (valid_page_no(no)) return &*pages[no-1];
    else return null;
}

static void load_page (Book& self, Page* page) {
    if (!page->texture) {
        page->load();
        self.estimated_page_memory += page->estimated_memory;
    }
}

static void unload_page (Book& self, Page* page) {
    if (page->texture) {
        page->unload();
        self.estimated_page_memory -= page->estimated_memory;
        DA(self.estimated_page_memory >= 0);
    }
}

bool Book::draw_if_needed () {
    if (!need_draw) return false;
    need_draw = false;
     // TODO: Currently we have a different context for each window, would it
     // be better to share a context between all windows?
    SDL_GL_MakeCurrent(window.sdl_window, window.gl_context);
     // Clear
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (Page* page = get_page(current_page_no)) {
        load_page(*this, page);
        page->last_viewed_at = uni::now();
        Vec scale;
         // Layout page
        switch (fit_mode) {
            case FIT: {
                 // Fit to screen
                 // slope = 1/aspect
                 // TODO: This seems to behave incorrectly sometimes
                zoom = slope(Vec(page->size)) > slope(Vec(window.size))
                    ? float(window.size.y) / page->size.y
                    : float(window.size.x) / page->size.x;
                scale = {zoom, zoom};
                offset = (window.size - page->size * scale) / 2;
                break;
            }
            case STRETCH: {
                 // Zoom isn't meaningful in stretch mode, but set it anyway
                zoom = slope(page->size) > slope(window.size)
                    ? float(window.size.y) / page->size.y
                    : float(window.size.x) / page->size.x;
                scale = window.size / page->size;
                offset = {0, 0};
                break;
            }
            case MANUAL: {
                 // Use whatever zoom and offset already are
                scale = {zoom, zoom};
                break;
            }
            default: AA(false);
        }
        Rect page_position = {offset, offset + page->size * scale};
         // Convert to OpenGL coords (-1,-1)..(+1,+1)
        Rect screen_rect = page_position / Vec(window.size) * float(2) - Vec(1, 1);
         // Draw
        page->draw(interpolation_mode, screen_rect);
    }
    SDL_GL_SwapWindow(window.sdl_window);
    return true;
}

void Book::next () {
    current_page_no += 1;
    if (!valid_page_no(current_page_no)) {
        current_page_no = pages.size();
    }
    fit_mode = app.settings->page.fit_mode;
    update_title(*this);
    need_draw = true;
}

void Book::prev () {
    current_page_no -= 1;
    if (!valid_page_no(current_page_no)) {
        current_page_no = 1;
    }
    fit_mode = app.settings->page.fit_mode;
    update_title(*this);
    need_draw = true;
}

void Book::seek (isize amount) {
    current_page_no = clamp(current_page_no + amount, 1, isize(pages.size()));
    fit_mode = app.settings->page.fit_mode;
    update_title(*this);
    need_draw = true;
}

void Book::set_fit_mode (FitMode mode) {
    fit_mode = mode;
    need_draw = true;
}

void Book::set_interpolation_mode (InterpolationMode mode) {
    interpolation_mode = mode;
    need_draw = true;
}

void Book::drag (Vec amount) {
    fit_mode = MANUAL;
    offset += amount;
    need_draw = true;
}

void Book::zoom_multiply (float factor) {
    if (Page* page = get_page(current_page_no)) {
        fit_mode = MANUAL;
         // Hacky way to zoom from center
        offset += page->size * zoom / 2;
        zoom *= factor;
        offset -= page->size * zoom / 2;
        need_draw = true;
    }
}

bool Book::is_fullscreen () {
    auto flags = AS(SDL_GetWindowFlags(window.sdl_window));
    return flags & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN);
}

void Book::set_fullscreen (bool fs) {
    AS(!SDL_SetWindowFullscreen(
        window.sdl_window,
        fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
    ));
    need_draw = true;
}

void Book::window_size_changed (IVec new_size) {
    new_size = new_size;
    need_draw = true;
}

bool Book::idle_processing () {
     // Preload pages forwards
    auto& memory_settings = app.settings->memory;
    for (uint32 i = 1; i <= memory_settings.preload_ahead; i++) {
        if (Page* page = get_page(current_page_no + i)) {
            if (!page->texture) {
                load_page(*this, page);
                return true;
            }
        }
    }
     // Preload pages backwards
    for (uint32 i = 1; i <= memory_settings.preload_behind; i++) {
        if (Page* page = get_page(current_page_no - i)) {
            if (!page->texture) {
                load_page(*this, page);
                return true;
            }
        }
    }
     // Unload pages if we're above the memory limit
    int64 limit = memory_settings.page_cache_mb * (1024*1024);
    if (estimated_page_memory > limit) {
        double oldest_viewed_at = INF;
        Page* oldest_page = null;
        for (isize no = 1; no <= isize(pages.size()); no++) {
             // Don't unload images in the preload region
            if (no >= current_page_no - memory_settings.preload_behind
             && no <= current_page_no + memory_settings.preload_ahead) {
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

} using namespace app;

#ifndef TAP_DISABLE_TESTS
#include "../base/ayu/resource.h"
#include "../base/glow/image.h"
#include "../base/tap/tap.h"

static tap::TestSet tests ("app/book", []{
    using namespace tap;

    IVec size = {120, 120};

    App app;
    app.hidden = true;
    app.settings->window.size = size;
    Book book (app, {
        ayu::file_resource_root() + "/base/glow/test/image.png"sv,
        ayu::file_resource_root() + "/base/glow/test/image2.png"sv
    });

    book.draw_if_needed();
    glow::Image img (size);
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.current_page_no, 1, "Initial page is 1");
    is(img[{60, 60}], glow::RGBA8(0x2674dbff), "First page is correct");

    book.next();
    book.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.current_page_no, 2, "Next page is 2");
    is(img[{60, 60}], glow::RGBA8(0x45942eff), "Second page is correct");

    book.next();
    book.draw_if_needed();
    is(book.current_page_no, 2, "Can't go past last page");

    book.prev();
    book.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.current_page_no, 1, "Go back to page 1");
    is(img[{60, 60}], glow::RGBA8(0x2674dbff), "Going back to first page works");

    book.prev();
    book.draw_if_needed();
    is(book.current_page_no, 1, "Can't go before page 1");

    is(img[{0, 0}], glow::RGBA8(0x00000000), "Default to fit mode");
    book.set_fit_mode(STRETCH);
    book.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{0, 0}], glow::RGBA8(0x2674dbff), "Stretch mode fills screen");

    done_testing();
});

#endif
