#include "layout.h"

#include "app.h"
#include "book.h"
#include "page-block.h"
#include "../dirt/ayu/reflection/describe.h"

namespace liv {

Spread::Spread (Book& book) {
    Vec small_align = book.state.settings->get(&LayoutSettings::small_align);
     // Collect visible pages
    for (i32 i : book.visible_range()) {
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

Projection::Projection (
    const BookState& state,
    const Spread& spread,
    Vec window_size
) {
    switch (state.settings->get(&LayoutSettings::orientation)) {
        case Direction::Up:
        case Direction::Down: size = window_size; break;
        case Direction::Left:
        case Direction::Right: size = {window_size.y, window_size.x}; break;
    }
    if (state.manual_offset) {
        expect(state.manual_zoom);
        expect(defined(*state.manual_zoom));
        zoom = *state.manual_zoom;
        expect(defined(*state.manual_offset));
        offset = *state.manual_offset;
    }
    else {
        if (state.manual_zoom) {
            expect(defined(*state.manual_zoom));
            zoom = *state.manual_zoom;
        }
        else if (!area(spread.size)) {
            zoom = 1;
        }
        else switch (state.settings->get(&LayoutSettings::auto_zoom_mode)) {
            case AutoZoomMode::Fit: {
                 // slope = 1 / aspect ratio
                if (slope(spread.size) > slope(size)) {
                    zoom = size.y / spread.size.y;
                }
                else {
                    zoom = size.x / spread.size.x;
                }
                zoom = spread.clamp_zoom(*state.settings, zoom);
                break;
            }
            case AutoZoomMode::FitWidth: {
                zoom = size.x / spread.size.x;
                zoom = spread.clamp_zoom(*state.settings, zoom);
                break;
            }
            case AutoZoomMode::FitHeight: {
                zoom = size.y / spread.size.y;
                zoom = spread.clamp_zoom(*state.settings, zoom);
                break;
            }
            case AutoZoomMode::Original: {
                zoom = 1;
                break;
            }
        }
         // Auto align
        Vec small_align = state.settings->get(&LayoutSettings::small_align);
        Vec large_align = state.settings->get(&LayoutSettings::large_align);
        Vec range = size - (spread.size * zoom); // Can be negative
        Vec align = {
            range.x > 0 ? small_align.x : large_align.x,
            range.y > 0 ? small_align.y : large_align.y
        };
        offset = range * align;
    }
}

void Projection::scroll (
    const Settings& settings, const Spread& spread, Vec amount
) {
     // Clamp to valid scroll area
    float scroll_margin = settings.get(&LayoutSettings::scroll_margin);
    Vec small_align = settings.get(&LayoutSettings::small_align);
     // Convert margin to pixels
    Vec margin_lt = size * scroll_margin;
    Vec margin_rb = size * (1 - scroll_margin);
     // Left side is constrained by right side of spread
    Vec valid_lt = margin_rb - spread.size * zoom;
     // Right side is constrained by left margin
    Vec valid_rb = margin_lt;
     // If the valid region is negative, the image is smaller than the margin
     // area, so just center it.
    if (valid_lt.x <= valid_rb.x) {
        offset.x = clamp(offset.x + amount.x, valid_lt.x, valid_rb.x);
    }
    else {
        offset.x = lerp(valid_lt.x, valid_rb.x, small_align.x);
    }
    if (valid_lt.y <= valid_rb.y) {
        offset.y = clamp(offset.y + amount.y, valid_lt.y, valid_rb.y);
    }
    else {
        offset.y = lerp(valid_lt.y, valid_rb.y, small_align.y);
    }
}

} using namespace liv;
