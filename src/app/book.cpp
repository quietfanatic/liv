#include "book.h"

#include <filesystem>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include "../base/geo/scalar.h"
#include "../base/glow/gl.h"
#include "../base/uni/time.h"
#include "app.h"
#include "page.h"

using namespace geo;
namespace fs = std::filesystem;

namespace app {

///// Private helpers

static void update_title (Book& self) {
    String title;
    if (self.pages.size() == 0) {
        title = "Little Image Viewer (nothing loaded)"s;
    }
    else {
        if (self.pages.size() > 1) {
            title = "[" + std::to_string(self.current_page_no)
                  + "/" + std::to_string(self.pages.size()) + "] ";
        }
        title += self.get_page(self.current_page_no)->filename;
         // In general, direct comparisons of floats are not good, but we do
         // slight snapping of our floats to half-integers, so this is fine.
        if (self.zoom != 1) {
            title += " (" + std::to_string(geo::round(self.zoom * 100)) + "%)";
        }
    }
    SDL_SetWindowTitle(self.window, title.c_str());
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

Book::Book (App& app, std::vector<String>&& filenames, String&& folder) :
    app(app),
    folder(std::move(folder)),
    auto_zoom_mode(app.setting(&PageSettings::auto_zoom_mode)),
    small_align(app.setting(&PageSettings::small_align)),
    large_align(app.setting(&PageSettings::large_align)),
    interpolation_mode(app.setting(&PageSettings::interpolation_mode)),
    window("Little Image Viewer", app.setting(&WindowSettings::size))
{
    SDL_SetWindowResizable(window, SDL_TRUE);
    DA(!SDL_GL_SetSwapInterval(1));

    if (app.setting(&WindowSettings::fullscreen)) {
        set_fullscreen(true);
    }
    glow::init();
    pages.reserve(filenames.size());
    for (auto& filename : filenames) {
        pages.emplace_back(std::make_unique<Page>(filename));
    }
    need_draw = true;
    if (!app.hidden) SDL_ShowWindow(window);
}
Book::~Book () { }


isize Book::clamp_page_no (isize no) {
    if (!pages.empty()) return clamp(no, 1, isize(pages.size()));
    else return 1;
}

Page* Book::get_page (isize no) {
    if (!pages.size()) return null;
    if (clamp_page_no(no) != no) return null;
    return &*pages[no-1];
}

isize Book::get_page_no_with_filename (Str filename) {
    for (isize i = 0; i < isize(pages.size()); i++) {
        if (pages[i]->filename == filename) return i + 1;
    }
    return 0;
}

///// Layout logic

float Book::clamp_zoom (float zoom) {
    auto max_zoom = app.setting(&PageSettings::max_zoom);
    auto min_page_size = app.setting(&PageSettings::min_page_size);
    if (Page* page = get_page(current_page_no)) {
        float min_zoom = min(1.f, min(
            min_page_size / page->size.x,
            min_page_size / page->size.y
        ));
        zoom = clamp(zoom, min_zoom, max_zoom);
    }
    else zoom = clamp(zoom, 1/max_zoom, max_zoom);
     // Slightly snap to half integers
    auto rounded = geo::round(zoom * 2) / 2;
    if (distance(zoom, rounded) < 0.0001) {
        zoom = rounded;
    }
    return zoom;
}

///// Controls

void Book::set_page (isize no) {
    current_page_no = clamp_page_no(no);
    if (app.setting(&PageSettings::reset_zoom_on_page_turn)) {
        manual_zoom = false;
        manual_offset = false;
    }
    need_draw = true;
}

void Book::set_align (Vec small, Vec large) {
    if (defined(small.x)) small_align.x = small.x;
    if (defined(small.y)) small_align.y = small.y;
    if (defined(large.x)) large_align.x = large.x;
    if (defined(large.y)) large_align.y = large.y;
    manual_offset = false;
    need_draw = true;
}

void Book::set_auto_zoom_mode (AutoZoomMode mode) {
    auto_zoom_mode = mode;
    manual_zoom = false;
    manual_offset = false;
    need_draw = true;
}

void Book::set_interpolation_mode (InterpolationMode mode) {
    interpolation_mode = mode;
    need_draw = true;
}

void Book::drag (Vec amount) {
    manual_offset = true;
    offset += amount;
    need_draw = true;
}

void Book::zoom_multiply (float factor) {
    if (Page* page = get_page(current_page_no)) {
        manual_zoom = true;
        if (manual_offset) {
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

void Book::reset_page () {
    auto_zoom_mode = app.setting(&PageSettings::auto_zoom_mode);
    small_align = app.setting(&PageSettings::small_align);
    large_align = app.setting(&PageSettings::large_align);
    interpolation_mode = app.setting(&PageSettings::interpolation_mode);
    manual_zoom = false;
    manual_offset = false;
    need_draw = true;
}

bool Book::is_fullscreen () {
    auto flags = AS(SDL_GetWindowFlags(window));
    return flags & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN);
}

void Book::set_fullscreen (bool fs) {
    AS(!SDL_SetWindowFullscreen(
        window,
        fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
    ));
}

///// Internal stuff

bool Book::draw_if_needed () {
    if (!need_draw) return false;
    need_draw = false;
     // TODO: Currently we have a different context for each window, would it
     // be better to share a context between all windows?
    SDL_GL_MakeCurrent(window, window.gl_context);
     // Clear
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (Page* page = get_page(current_page_no)) {
        load_page(*this, page);
        page->last_viewed_at = uni::now();
         // Determine layout
        Vec window_size = get_window_size();
        if (!manual_offset) {
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
        page->draw(interpolation_mode, zoom, screen_rect);
    }
    update_title(*this);
    SDL_GL_SwapWindow(window);
    return true;
}

bool Book::idle_processing () {
     // Preload pages forwards
    int32 preload_ahead = app.setting(&MemorySettings::preload_ahead);
    int32 preload_behind = app.setting(&MemorySettings::preload_behind);
    int32 page_cache_mb = app.setting(&MemorySettings::page_cache_mb);
    for (int32 i = 1; i <= preload_ahead; i++) {
        if (Page* page = get_page(current_page_no + i)) {
            if (!page->texture && !page->load_failed) {
                load_page(*this, page);
                return true;
            }
        }
    }
     // Preload pages backwards
    for (int32 i = 1; i <= preload_behind; i++) {
        if (Page* page = get_page(current_page_no - i)) {
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
            if (no >= current_page_no - preload_behind
             && no <= current_page_no + preload_ahead) {
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
    SDL_GL_GetDrawableSize(window, &w, &h);
    AA(w > 0 && h > 0);
    return {w, h};
}

void Book::window_size_changed (IVec size) {
    AA(size.x > 0 && size.y > 0);
    glViewport(0, 0, size.x, size.y);
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
    Book book (app, {
        exe_folder + "/res/base/glow/test/image.png"sv,
        exe_folder + "/res/base/glow/test/image2.png"sv
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
