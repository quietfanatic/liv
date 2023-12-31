#pragma once

#include "../dirt/geo/range.h"
#include "../dirt/uni/common.h"
#include "common.h"
#include "layout.h"
#include "page.h"
#include "settings.h"

namespace liv {

struct BookState {
    explicit BookState (Book* book);

     // Object parent.  This has a link to Settings and BookSource.
    Book* book;
     // Indexes of pages being currently viewed.
    IRange spread_range;
     // page_indexes clamped to valid page indexes.
    IRange visible_range () const;
     // View parameters
    LayoutParams layout_params;
    RenderParams render_params;

    ///// Controls
     // Takes a 1-based page number.  page_indexes will be set to
     //     {no - 1, no + spread_count - 1}
     // but clamped such that there is at least one visible page.
    void set_page_number (int32 no);
    int32 get_page_number () const { return spread_range.l + 1; }

     // Set number of pages to view simultaneously.  Clamps to 1..2048
     // TODO: clamp smaller wow
    void set_spread_count (int32 count);

    void set_window_background (Fill);
    void set_auto_zoom_mode (AutoZoomMode);
    void set_align (geo::Vec small, geo::Vec large);
    void set_interpolation_mode (InterpolationMode);
     // Adds amount to view.offset
    void drag (geo::Vec amount);

    void zoom_multiply (float factor);

     // Reset all layout parameters
    void reset_layout ();
};

} // namespace liv
