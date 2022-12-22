#include "layout.h"

#include "app.h"
#include "book.h"
#include "page-block.h"

namespace app {

LayoutParams::LayoutParams (const Settings* settings) :
    spread_direction(settings->get(&LayoutSettings::spread_direction)),
    auto_zoom_mode(settings->get(&LayoutSettings::auto_zoom_mode)),
    small_align(settings->get(&LayoutSettings::small_align)),
    large_align(settings->get(&LayoutSettings::large_align))
{ }

Spread::Spread (PageBlock& block, IRange viewing, const LayoutParams& params) {
     // Collect visible pages
    for (int32 i = viewing.l; i < viewing.r; i++) {
        if (Page* page = block.get(i)) {
            block.load_page(page);
            pages.emplace_back(block.get(i));
        }
    }
    size = {0, 0};
    switch (params.spread_direction) {
        case RIGHT: {
             // Set height to height of tallest page
            for (auto& p : pages) {
                if (p.page->size.y > size.y) size.y = p.page->size.y;
            }
            for (auto& p : pages) {
                 // Accumulate width
                p.offset.x = size.x;
                size.x += p.page->size.x;
                 // Align vertically
                p.offset.y = (size.y - p.page->size.y) * params.small_align.y;
            }
            break;
        }
        case LEFT: {
            for (auto& p : pages) {
                if (p.page->size.y > size.y) size.y = p.page->size.y;
            }
            for (auto it = pages.rbegin(); it != pages.rend(); it++) {
                auto& p = *it;
                p.offset.x = size.x;
                size.x += p.page->size.x;
                p.offset.y = (size.y - p.page->size.y) * params.small_align.y;
            }
            break;
        }
        case DOWN: {
            for (auto& p : pages) {
                if (p.page->size.x > size.x) size.x = p.page->size.x;
            }
            for (auto& p : pages) {
                p.offset.y = size.y;
                size.y += p.page->size.y;
                p.offset.x = (size.x - p.page->size.x) * params.small_align.x;
            }
            break;
        }
        case UP: {
            for (auto& p : pages) {
                if (p.page->size.x > size.x) size.x = p.page->size.x;
            }
            for (auto it = pages.rbegin(); it != pages.rend(); it++) {
                auto& p = *it;
                p.offset.y = size.y;
                size.y += p.page->size.y;
                p.offset.x = (size.x - p.page->size.x) * params.small_align.x;
            }
            break;
        }
    }
};

float Spread::clamp_zoom (const Settings* settings, float zoom) const {
    if (!defined(zoom)) return 1;
     // Slightly snap to half integers
    auto rounded = geo::round(zoom * 2) / 2;
    if (distance(zoom, rounded) < 0.0001) {
        zoom = rounded;
    }
     // Now clamp
    auto max_zoom = settings->get(&LayoutSettings::max_zoom);
    auto min_size = settings->get(&LayoutSettings::min_zoomed_size);
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
    const Settings* settings,
    const Spread& spread,
    const LayoutParams& params,
    Vec window_size
) {
    if (defined(params.manual_offset)) {
        offset = params.manual_offset;
         // If offset is defined zoom should be too.
        zoom = params.manual_zoom;
    }
    else {
        if (defined(params.manual_zoom)) {
            zoom = params.manual_zoom;
        }
        else if (!area(spread.size)) {
            zoom = 1;
        }
        else {
            switch (params.auto_zoom_mode) {
                case FIT: {
                     // slope = 1 / aspect ratio
                    if (slope(spread.size) > slope(window_size)) {
                        zoom = window_size.y / spread.size.y;
                    }
                    else {
                        zoom = window_size.x / spread.size.x;
                    }
                    zoom = spread.clamp_zoom(settings, zoom);
                    break;
                }
                case FIT_WIDTH: {
                    zoom = spread.clamp_zoom(settings, window_size.x / spread.size.x);
                    break;
                }
                case FIT_HEIGHT: {
                    zoom = spread.clamp_zoom(settings, window_size.y / spread.size.y);
                    break;
                }
                case ORIGINAL: {
                    zoom = 1;
                    break;
                }
            }
        }
         // Auto align
        Vec range = window_size - (spread.size * zoom); // Can be negative
        Vec align = {
            range.x > 0 ? params.small_align.x : params.large_align.x,
            range.y > 0 ? params.small_align.y : params.large_align.y
        };
        offset = range * align;
    }
}

} // namespace app
