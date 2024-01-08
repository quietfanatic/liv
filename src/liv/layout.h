#pragma once

#include "../dirt/geo/range.h"
#include "../dirt/geo/vec.h"
#include "common.h"
#include "settings.h"

namespace liv {

struct LayoutParams {
    Direction spread_direction;
    AutoZoomMode auto_zoom_mode;
     // Controls alignment of pages when they're smaller than the window.
     // (0, 0) means the page's top-left corner is in the top-left corner of
     // the window.  (1, 1) means the page's bottom-right corner is in the
     // bottom-right corner of the window.  (0.5, 0.5) means the page's center
     // is in the center of the window.
    Vec small_align;
     // Controls alignment of the page when it's larger than the window.
    Vec large_align;
     // NAN means no manual zoom/offset is applied, so use auto_zoom_mode.
    float manual_zoom = GNAN;
    Vec manual_offset = GNAN;
    LayoutParams () = default;
    LayoutParams (const LayoutParams&) = default;
    LayoutParams (const Settings*);
};

// Everything in Spread* uses spread coordinates, with pixels the same size as
// page coordinates, without zoom or offset applied.
struct SpreadPage {
    Page* page;
    Vec offset = GNAN;
};

struct Spread {
    UniqueArray<SpreadPage> pages;
    Vec size;
     // Uses small_align and large_align.  PageBlock is not const because it'll
     // be sent load_page messages.
    Spread (PageBlock&, IRange viewing_pages, const LayoutParams&);
     // Uses max_zoom and min_page_size
    float clamp_zoom (const Settings*, float) const;
};

// This determines how a Spread is shown in the window.
struct Layout {
     // Zoom is applied before offset
    float zoom;
     // Offset is applied after zoom
    Vec offset;
    Layout (
        const Settings*, const Spread&, const LayoutParams&, Vec window_size
    );
};

} // namespace liv
