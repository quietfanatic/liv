#pragma once

#include "../dirt/geo/range.h"
#include "../dirt/geo/vec.h"
#include "common.h"
#include "settings.h"

namespace liv {

// Everything in Spread* uses spread coordinates, with pixels the same size as
// page coordinates, without zoom or offset applied.
struct SpreadPage {
    Page* page;
    Vec offset = GNAN;
};

struct Spread {
    UniqueArray<SpreadPage> pages;
    Vec size;
     // Uses small_align and large_align.  Book is not const because we'll load
     // pages on demand.
    Spread (Book&);
     // Uses max_zoom and min_page_size
    float clamp_zoom (const Settings&, float) const;
};

// This determines how a Spread is shown in the window.
struct Layout {
     // Zoom is applied before offset
    float zoom;
     // Offset is in window coordinates, not spread coordinates
    Vec offset;
     // Window size, but swapped if orientation is Left or Right
    Vec size;
    Layout (
        const BookState&, const Spread&, Vec window_size
    );

     // Changes offset, clamped according to scroll_margin setting.  This does
     // not require recalculating the entire layout.
    void scroll (const Settings&, const Spread&, Vec);
};

} // namespace liv
