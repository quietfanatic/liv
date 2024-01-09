#pragma once

#include "../dirt/geo/range.h"
#include "../dirt/uni/common.h"
#include "common.h"
#include "layout.h"
#include "page.h"
#include "settings.h"

namespace liv {

 // Holds all the mutable state associated with a book.
struct BookState {
    BookState () = default;
    BookState (BookState&&) = default;
    BookState& operator= (BookState&&) = default;
    explicit BookState (std::unique_ptr<Settings>);

     // Book-specific settings.  Has the app settings as its parent.
    std::unique_ptr<Settings> settings;
     // Index of first page currently being viewed.
    int32 page_offset = 0;
     // If not defined, use the auto zoom mode.
    std::optional<float> manual_zoom;
    std::optional<Vec> manual_offset;

     // Pages currently being viewed, clamped to valid page indexes.
    IRange viewing_range () const;

    void set_auto_zoom_mode (AutoZoomMode);
    void set_align (geo::Vec small, geo::Vec large);

     // Reset all layout parameters except spread_count
    void reset_layout ();
};

} // namespace liv
