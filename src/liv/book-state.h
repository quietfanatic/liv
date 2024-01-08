#pragma once

#include "../dirt/geo/range.h"
#include "../dirt/uni/common.h"
#include "common.h"
#include "layout.h"
#include "page.h"
#include "settings.h"

namespace liv {

struct BookState {
    explicit BookState (Book*, std::unique_ptr<Settings>);

     // Object parent.  This has a link to BookSource.
    Book* book;
     // Book-specific settings.  Has the app settings as its parent.
    std::unique_ptr<Settings> settings;
     // Indexes of pages being currently viewed.
    IRange spread_range;
     // spread_range clamped to valid page indexes.
    IRange visible_range () const;
     // View parameters
    LayoutParams layout_params;

    ///// Controls
     // Takes a 0-based page number.  spread_range will be set to
     //     {no, no + spread_count}
     // but clamped such that there is at least one visible page.
    void set_page_offset (int32 no);

     // Set number of pages to view simultaneously.  Clamps to 1..2048
     // TODO: clamp smaller wow
    void set_spread_count (int32 count);

    void set_auto_zoom_mode (AutoZoomMode);
    void set_align (geo::Vec small, geo::Vec large);
     // Adds amount to view.offset
    void drag (geo::Vec amount);

    void zoom_multiply (float factor);

     // Reset all layout parameters
    void reset_layout ();
};

} // namespace liv
