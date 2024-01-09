#include "layout.h"

#include "app.h"
#include "book.h"
#include "page-block.h"
#include "../dirt/ayu/reflection/describe.h"

namespace liv {

Spread::Spread (Book& book) {
    Vec small_align = book.state.settings->get(&LayoutSettings::small_align);
     // Collect visible pages
    for (int32 i : book.visible_range()) {
        if (Page* page = book.block.get(i)) {
            book.block.load_page(page);
            pages.emplace_back(book.block.get(i));
        }
    }
    size = {0, 0};
    switch (book.state.settings->get(&LayoutSettings::spread_direction)) {
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
};

float Spread::clamp_zoom (const Settings& settings, float zoom) const {
    if (!defined(zoom)) return 1;
     // Slightly snap to half integers
    auto rounded = geo::round(zoom * 2) / 2;
    if (distance(zoom, rounded) < 0.0001) {
        zoom = rounded;
    }
     // Now clamp
    auto max_zoom = settings.get(&LayoutSettings::max_zoom);
    auto min_size = settings.get(&LayoutSettings::min_zoomed_size);
    if (size) {
        float min_zoom = min(1.f, min(
            min_size / size.x,
            min_size / size.y
        ));
        zoom = clamp(zoom, min_zoom, max_zoom);
    }
    else zoom = clamp(zoom, 1/max_zoom, max_zoom);
    require(defined(zoom));
    return zoom;
}

Layout::Layout (
    const BookState& state,
    const Spread& spread,
    Vec window_size
) {
    if (defined(state.manual_offset)) {
        offset = state.manual_offset;
         // If offset is defined zoom should be too.
        zoom = state.manual_zoom;
    }
    else {
        if (defined(state.manual_zoom)) {
            zoom = state.manual_zoom;
        }
        else if (!area(spread.size)) {
            zoom = 1;
        }
        else {
            switch (state.settings->get(&LayoutSettings::auto_zoom_mode)) {
                case AutoZoomMode::Fit: {
                     // slope = 1 / aspect ratio
                    if (slope(spread.size) > slope(window_size)) {
                        zoom = window_size.y / spread.size.y;
                    }
                    else {
                        zoom = window_size.x / spread.size.x;
                    }
                    zoom = spread.clamp_zoom(*state.settings, zoom);
                    break;
                }
                case AutoZoomMode::FitWidth: {
                    zoom = window_size.x / spread.size.x;
                    zoom = spread.clamp_zoom(*state.settings, zoom);
                    break;
                }
                case AutoZoomMode::FitHeight: {
                    zoom = window_size.y / spread.size.y;
                    zoom = spread.clamp_zoom(*state.settings, zoom);
                    break;
                }
                case AutoZoomMode::Original: {
                    zoom = 1;
                    break;
                }
            }
        }
         // Auto align
        Vec small_align = state.settings->get(&LayoutSettings::small_align);
        Vec large_align = state.settings->get(&LayoutSettings::large_align);
        Vec range = window_size - (spread.size * zoom); // Can be negative
        Vec align = {
            range.x > 0 ? small_align.x : large_align.x,
            range.y > 0 ? small_align.y : large_align.y
        };
        offset = range * align;
    }
}

} using namespace liv;
