#pragma once

#include "../dirt/uni/common.h"
#include "../dirt/wind/window.h"
#include "common.h"
#include "layout.h"

namespace liv {

 // Responsible for window management and drawing.
struct BookView {
    explicit BookView (Book* book);
    ~BookView ();

    void update_spread () { need_something = need_spread = true; }
    void update_projection () { need_something = need_projection = true; }
    void update_picture () { need_something = need_picture = true; }

     // Lazy getters.
    const Spread& get_spread ();
    const Projection& get_projection ();

    bool is_fullscreen () const;
    void set_fullscreen (bool);
    bool is_minimized () const;
    geo::IVec get_window_size () const;
    void window_size_changed (geo::IVec new_size);
     // Returns true if drawing was actually done.
    bool draw_if_needed ();

     // Object parent.  TODO: pass this instead of storing it, or maybe even
     // reverse-member-lookup it.
    Book* book;

    wind::Window window;

    Spread spread;
    Projection projection;

     // True if any of the below are true
    bool need_something = true;
     // Pages being viewed have changed
    bool need_spread = true;
     // Zoom, offset have changed
    bool need_projection = true;
     // Window title needs updating
    bool need_title = true;
     // Don't regenerate anything, just redraw
    bool need_picture = true;
};

} // namespace liv
