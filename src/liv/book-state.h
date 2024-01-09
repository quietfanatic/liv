#pragma once

#include "../dirt/geo/range.h"
#include "../dirt/uni/common.h"
#include "common.h"
#include "layout.h"
#include "page.h"
#include "settings.h"

namespace liv {

struct BookState {
    explicit BookState (std::unique_ptr<Settings>);

     // Book-specific settings.  Has the app settings as its parent.
    std::unique_ptr<Settings> settings;
     // Index of first page currently being viewed.
    int32 page_offset = 0;
     // NAN means no manual zoom/offset, is applied, so use auto_zoom.
    float manual_zoom = GNAN;
    Vec manual_offset = GNAN;

     // Pages currently being viewed, clamped to valid page indexes.
    IRange viewing_range () const;

     // Takes a 0-based page number.  spread_range will be set to
     //     {no, no + spread_count}
     // but clamped such that there is at least one visible page.
    void set_page_offset (int32, const PageBlock&);

     // Set number of pages to view simultaneously.  Clamps to 1..2048
     // TODO: clamp smaller wow
    void set_spread_count (int32 count, const PageBlock&);

    void set_auto_zoom_mode (AutoZoomMode);
    void set_align (geo::Vec small, geo::Vec large);
     // Adds amount to view.offset
    void drag (geo::Vec amount, BookView&);

    void zoom_multiply (float factor, BookView&);

     // Reset all layout parameters
    void reset_layout ();
};

} // namespace liv
