#include "book.h"

#include <filesystem>
#include <SDL2/SDL.h>
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

///// Private helpers

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
    SDL_SetWindowTitle(self.sdl_window, title.c_str());
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

///// Contents

Book::Book (App& app, Str folder) :
    app(app),
    folder(folder)
{
    AA(false);
}
Book::Book (App& app, const std::vector<String>& filenames) :
    app(app),
    auto_zoom_mode(app.settings->page.auto_zoom_mode),
    small_align(app.settings->page.small_align),
    large_align(app.settings->page.large_align),
    interpolation_mode(app.settings->page.interpolation_mode)
{
    AS(!SDL_InitSubSystem(SDL_INIT_VIDEO));
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    sdl_window = AS(SDL_CreateWindow(
        "Little Image Viewer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        app.settings->window.size.x,
        app.settings->window.size.y,
        SDL_WINDOW_OPENGL | (app.hidden ? SDL_WINDOW_HIDDEN : 0)
    ));
    SDL_SetWindowResizable(sdl_window, SDL_TRUE);
    gl_context = AS(SDL_GL_CreateContext(sdl_window));
    DA(!SDL_GL_SetSwapInterval(1));

    if (app.settings->window.fullscreen) {
        set_fullscreen(true);
    }
    glow::init();
    pages.reserve(filenames.size());
    for (auto& filename : filenames) {
        pages.emplace_back(std::make_unique<Page>(filename));
    }
    update_title(*this);
    need_draw = true;
}
Book::~Book () { }


isize Book::clamp_page_no (isize no) {
    if (!pages.empty()) return clamp(no, 1, isize(pages.size()));
    else return 1;
}

Page* Book::get_page (isize no) {
    if (clamp_page_no(no) == no) return &*pages[no-1];
    else return null;
}

///// Layout logic

float Book::clamp_zoom (float zoom) {
    auto max_zoom = app.settings->page.max_zoom;
    auto min_page_size = app.settings->page.min_page_size;
    if (Page* page = get_page(current_page_no)) {
        float min_zoom = min(1.f, min(
            min_page_size / page->size.x,
            min_page_size / page->size.y
        ));
        return clamp(zoom, min_zoom, max_zoom);
    }
    else return clamp(zoom, 1/max_zoom, max_zoom);
}

///// Controls

void Book::set_page (isize no) {
    current_page_no = clamp_page_no(no);
    if (app.settings->page.reset_zoom_on_page_turn) {
        manual_zoom = false;
        manual_align = false;
    }
    update_title(*this);
    need_draw = true;
}

void Book::set_align (Vec small, Vec large) {
    if (defined(small.x)) small_align.x = small.x;
    if (defined(small.y)) small_align.y = small.y;
    if (defined(large.x)) large_align.x = large.x;
    if (defined(large.y)) large_align.y = large.y;
    if (!manual_align) need_draw = true;
}

void Book::set_auto_zoom_mode (AutoZoomMode mode) {
    auto_zoom_mode = mode;
    manual_zoom = false;
    manual_align = false;
    need_draw = true;
}

void Book::set_interpolation_mode (InterpolationMode mode) {
    interpolation_mode = mode;
    need_draw = true;
}

void Book::drag (Vec amount) {
    manual_align = true;
    offset += amount;
    need_draw = true;
}

void Book::zoom_multiply (float factor) {
    if (Page* page = get_page(current_page_no)) {
        manual_zoom = true;
        if (manual_align) {
             // Hacky way to zoom from center
             // TODO: zoom from center of window, not center of page
            offset += page->size * zoom / 2;
            zoom = clamp_zoom(zoom * factor);
            offset -= page->size * zoom / 2;
        }
        else {
            zoom = clamp_zoom(zoom * factor);
        }
        need_draw = true;
    }
}

bool Book::is_fullscreen () {
    auto flags = AS(SDL_GetWindowFlags(sdl_window));
    return flags & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN);
}

void Book::set_fullscreen (bool fs) {
    AS(!SDL_SetWindowFullscreen(
        sdl_window,
        fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
    ));
    need_draw = true;
}

///// Internal stuff

bool Book::draw_if_needed () {
    if (!need_draw) return false;
    need_draw = false;
     // TODO: Currently we have a different context for each window, would it
     // be better to share a context between all windows?
    SDL_GL_MakeCurrent(sdl_window, gl_context);
     // Clear
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (Page* page = get_page(current_page_no)) {
        load_page(*this, page);
        page->last_viewed_at = uni::now();
         // Determine layout
        Vec window_size = get_window_size();
        if (!manual_align) {
            if (!manual_zoom) {
                switch (auto_zoom_mode) {
                    case FIT: {
                         // slope = 1 / aspect ratio
                        if (slope(Vec(page->size)) > slope(window_size)) {
                            zoom = window_size.y / page->size.y;
                        }
                        else {
                            zoom = window_size.x / page->size.x;
                        }
                        zoom = clamp_zoom(zoom);
                        break;
                    }
                    case FIT_WIDTH: {
                        zoom = clamp_zoom(window_size.x / page->size.x);
                        break;
                    }
                    case FIT_HEIGHT: {
                        zoom = clamp_zoom(window_size.y / page->size.y);
                        break;
                    }
                    case FILL: {
                         // slope = 1 / aspect ratio
                        if (slope(Vec(page->size)) > slope(window_size)) {
                            zoom = float(window_size.x) / page->size.x;
                        }
                        else {
                            zoom = float(window_size.y) / page->size.y;
                        }
                        zoom = clamp_zoom(zoom);
                        break;
                    }
                    case ORIGINAL: {
                        zoom = 1;
                        break;
                    }
                }
            }
             // Auto align
            Vec range = window_size - (page->size * zoom); // Can be negative
            offset.x = range.x * (range.x > 0 ? small_align.x : large_align.x);
            offset.y = range.y * (range.y > 0 ? small_align.y : large_align.y);
        }
        Rect page_position = {offset, offset + page->size * zoom};
         // Convert to OpenGL coords (-1,-1)..(+1,+1)
        Rect screen_rect = page_position / Vec(window_size) * float(2) - Vec(1, 1);
         // Draw
        page->draw(interpolation_mode, screen_rect);
    }
    SDL_GL_SwapWindow(sdl_window);
    return true;
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

IVec Book::get_window_size () {
    int w, h;
    SDL_GL_GetDrawableSize(sdl_window, &w, &h);
    AA(w > 0 && h > 0);
    return {w, h};
}

void Book::window_size_changed (IVec) {
    need_draw = true;
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

    is(img[{0, 60}], glow::RGBA8(0x2674dbff), "Default to auto_zoom_mode = fit");
    book.set_auto_zoom_mode(ORIGINAL);
    book.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{0, 60}], glow::RGBA8(0x000000ff), "auto_zoom_mode = original");

    done_testing();
});

#endif
