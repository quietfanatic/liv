#include "book-view.h"

#include <SDL2/SDL_video.h>
#include "../dirt/uni/time.h"
#include "book.h"

namespace liv {

BookView::BookView (Book* book) :
    book(book),
    window(
        "Little Image Viewer",
        book->state.settings->get(&WindowSettings::size)
    )
{
    plog("created window");
    SDL_SetWindowResizable(window, SDL_TRUE);
    expect(!SDL_GL_SetSwapInterval(1));
    if (book->state.settings->get(&WindowSettings::fullscreen)) {
        window.set_fullscreen(true);
    }
    plog("set window props");
    glow::init();
    plog("fetched gl functions");
    if (!book->state.settings->get(&WindowSettings::hidden)) {
        SDL_ShowWindow(window);
    }
    plog("showed window");
}

BookView::~BookView () { }

Vec BookView::get_picture_size () {
    if (!need_picture_size) return picture_size;
    Vec window_size = window.size();
    switch (book->state.settings->get(&LayoutSettings::orientation)) {
        case Direction::Up:
        case Direction::Down: picture_size = window_size; break;
        case Direction::Left:
        case Direction::Right:
            picture_size = {window_size.y, window_size.x}; break;
    }
    need_picture_size = false;
    return picture_size;
}

void gen_spread (BookView& self) {
    auto pages = UniqueArray<PageView>(Capacity(size(self.book->visible_range())));
    Vec size = {0, 0};
    auto& state = self.book->state;
    auto& block = self.book->block;
    Vec small_align = state.settings->get(&LayoutSettings::small_align);
     // Collect visible pages
    for (i32 i : self.book->visible_range()) {
        if (Page* page = block.get(i)) {
            block.load_page(page);
            pages.emplace_back_expect_capacity(page, GNAN);
        }
    }
    switch (state.settings->get(&LayoutSettings::spread_direction)) {
        case Direction::Right: {
             // Set height to height of tallest page
            for (auto& p : pages) {
                if (p.page->size.y > size.y) size.y = p.page->size.y;
            }
            for (auto& p : pages) {
                 // Accumulate width
                p.offset.x = size.x;
                size.x += p.page->size.x;
                 // Align vertically
                p.offset.y = (size.y - p.page->size.y) * small_align.y;
            }
            break;
        }
        case Direction::Left: {
            for (auto& p : pages) {
                if (p.page->size.y > size.y) size.y = p.page->size.y;
            }
            for (auto it = pages.rbegin(); it != pages.rend(); it++) {
                auto& p = *it;
                p.offset.x = size.x;
                size.x += p.page->size.x;
                p.offset.y = (size.y - p.page->size.y) * small_align.y;
            }
            break;
        }
        case Direction::Down: {
            for (auto& p : pages) {
                if (p.page->size.x > size.x) size.x = p.page->size.x;
            }
            for (auto& p : pages) {
                p.offset.y = size.y;
                size.y += p.page->size.y;
                p.offset.x = (size.x - p.page->size.x) * small_align.x;
            }
            break;
        }
        case Direction::Up: {
            for (auto& p : pages) {
                if (p.page->size.x > size.x) size.x = p.page->size.x;
            }
            for (auto it = pages.rbegin(); it != pages.rend(); it++) {
                auto& p = *it;
                p.offset.y = size.y;
                size.y += p.page->size.y;
                p.offset.x = (size.x - p.page->size.x) * small_align.x;
            }
            break;
        }
    }
    self.need_spread = false;
    self.spread_size = size;
    swap(self.pages, pages); // tail call deleter
}

Slice<PageView> BookView::get_pages () {
    if (need_spread) gen_spread(*this);
    return pages;
}

Vec BookView::get_spread_size () {
    if (need_spread) gen_spread(*this);
    return spread_size;
}

float BookView::get_zoom () {
    if (!need_zoom) return zoom;
    auto& state = book->state;
    if (state.manual_zoom) {
        expect(defined(*state.manual_zoom));
        zoom = *state.manual_zoom;
    }
    else {
        auto mode = state.settings->get(&LayoutSettings::auto_zoom_mode);
        if (mode == AutoZoomMode::Original) zoom = 1;
        else {
            Vec ss = get_spread_size();
            if (!area(ss)) {
                zoom = 1;
            }
            else {
                auto ps = get_picture_size();
                switch (state.settings->get(&LayoutSettings::auto_zoom_mode)) {
                    case AutoZoomMode::Fit: {
                         // slope = 1 / aspect ratio
                        if (slope(ss) > slope(ps)) {
                            zoom = ps.y / ss.y;
                        }
                        else {
                            zoom = ps.x / ss.x;
                        }
                        zoom = clamp_zoom(zoom);
                        break;
                    }
                    case AutoZoomMode::FitWidth: {
                        zoom = ps.x / ss.x;
                        zoom = clamp_zoom(zoom);
                        break;
                    }
                    case AutoZoomMode::FitHeight: {
                        zoom = ps.y / ss.y;
                        zoom = clamp_zoom(zoom);
                        break;
                    }
                    default: never();
                }
            }
        }
    }
    need_zoom = false;
    return zoom;
}

Vec BookView::get_offset () {
    if (!need_offset) return offset;
    auto& state = book->state;
    if (state.manual_offset) {
        expect(defined(*state.manual_offset));
        offset = *state.manual_offset;
    }
    else {
         // Auto align
        Vec ps = get_picture_size();
        Vec ss = get_spread_size();
        Vec small_align = state.settings->get(&LayoutSettings::small_align);
        Vec large_align = state.settings->get(&LayoutSettings::large_align);
        Vec range = ps - (ss * zoom); // Can be negative
        Vec align = {
            range.x > 0 ? small_align.x : large_align.x,
            range.y > 0 ? small_align.y : large_align.y
        };
        offset = range * align;
    }
    need_offset = false;
    return offset;
}

float BookView::clamp_zoom (float req) {
    if (!defined(req)) return 1;
     // Slightly snap to half integers
    auto rounded = geo::round(req * 2) / 2;
    if (distance(req, rounded) < 0.0001) {
        req = rounded;
    }
     // Now clamp
    auto max_zoom = book->state.settings->get(&LayoutSettings::max_zoom);
    auto min_size = book->state.settings->get(&LayoutSettings::min_zoomed_size);
    if (auto ss = get_spread_size()) {
        float min_zoom = min(1.f, min(
            min_size / ss.x,
            min_size / ss.y
        ));
        zoom = clamp(req, min_zoom, max_zoom);
    }
    else zoom = clamp(req, 1/max_zoom, max_zoom);
    require(defined(zoom));
    return zoom;
}

Vec BookView::clamp_offset (Vec req) {
     // Clamp to valid scroll area
    Vec ps = get_picture_size();
    Vec ss = get_spread_size();
    float scroll_margin = book->state.settings->get(&LayoutSettings::scroll_margin);
    Vec small_align = book->state.settings->get(&LayoutSettings::small_align);
     // Convert margin to pixels
    Vec margin_lt = ps * scroll_margin;
    Vec margin_rb = ps * (1 - scroll_margin);
     // Left side is constrained by right side of spread
    Vec valid_lt = margin_rb - ss * zoom;
     // Right side is constrained by left margin
    Vec valid_rb = margin_lt;
     // If the valid region is negative, the image is smaller than the valid
     // area, so ignore the requested offset coord and use small_align.
    Vec r;
    if (valid_lt.x <= valid_rb.x) {
        r.x = clamp(req.x, valid_lt.x, valid_rb.x);
    }
    else {
        r.x = lerp(valid_lt.x, valid_rb.x, small_align.x);
    }
    if (valid_lt.y <= valid_rb.y) {
        r.y = clamp(req.y, valid_lt.y, valid_rb.y);
    }
    else {
        r.y = lerp(valid_lt.y, valid_rb.y, small_align.y);
    }
    return r;
}

bool BookView::draw_if_needed () {
    if (!need_title & !need_picture) return false;
    if (need_title) {
         // Theoretically we track whether we need to do the title independently
         // of whether we need to draw.
        AnyString title;
        IRange visible = book->visible_range();
        if (book->block.count() == 0) {
            title = "Little Image Viewer (nothing loaded)";
        }
        else if (empty(visible)) {
            title = "Little Image Viewer (no pages visible)";
        }
        else {
            auto& title_format = book->state.settings->get(&WindowSettings::title);
            UniqueString t;
            title_format.write(t, book);
            title = t;
        }
         // This might be an X-specific problem, but if SDL_SetWindowTitle is given
         // invalid Unicode, the window title doesn't get updated.  There's no way
         // to check that this happened, because the string returned by
         // SDL_GetWindowTitle is the requested title, not the string that's
         // currently being rendered on the title bar.  Checking the validity of the
         // Unicode ahead of time would require having access to a table of hundreds
         // of thousands of characters.  So the only thing we can really do is to
         // set the error message title, then set the desired title, and if the
         // desired title has invalid unicode, the old error title will remain
         // rendered.
        SDL_SetWindowTitle(window, "Little Image Viewer (invalid unicode in title)");
        SDL_SetWindowTitle(window, title.c_str());
        need_title = false;
    }
    if (need_picture) {
         // TODO: Currently we have a different context for each window, would it
         // be better to share a context between all windows?  Not that we currently
         // allow multiple windows per process.
        SDL_GL_MakeCurrent(window, window.gl_context);
         // Draw background
        auto bg = book->state.settings->get(&RenderSettings::window_background);
        glClearColor(
            bg.r / 255.f,
            bg.g / 255.f,
            bg.b / 255.f,
            bg.a / 255.f // Alpha is probably ignored
        );
        glClear(GL_COLOR_BUFFER_BIT);
         // Draw spread
        Vec picture_size = get_picture_size();
        auto spread_pages = get_pages();
        float zoom = get_zoom();
        Vec offset = get_offset();
        for (auto& spread_page : spread_pages) {
            spread_page.page->last_viewed_at = uni::now();
            Rect spread_rect = Rect(
                spread_page.offset,
                spread_page.offset + spread_page.page->size
            );
            Rect window_rect = spread_rect * zoom + offset;
             // Convert to OpenGL coords (-1,-1)..(+1,+1)
            Rect screen_rect = window_rect / picture_size * float(2) - Vec(1, 1);
             // Draw
            spread_page.page->draw(*book->state.settings, zoom, screen_rect);
        }
        plog("drew view");
         // vsync
        SDL_GL_SwapWindow(window);
        plog("swapped window");
        need_picture = false;
    }
    return true;
}

void BookView::window_size_changed (IVec size) {
     // TODO: write window.size setting
    require(size.x > 0 && size.y > 0);
    glViewport(0, 0, size.x, size.y);
    update_picture_size();
}

} // namespace liv
