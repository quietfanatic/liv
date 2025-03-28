#pragma once

#include "../dirt/uni/common.h"
#include "../dirt/wind/window.h"
#include "common.h"

namespace liv {

struct PageView {
    Page* page;
    Vec offset;  // unzoomed coordinates
};

 // Responsible for window management and drawing.
struct BookView {
    explicit BookView (Book* book);
    ~BookView ();

     // Object parent.  TODO: pass this instead of storing it, or maybe even
     // reverse-member-lookup it.
    Book* book;

    wind::Window window;

     // View properties.  These could be stale, so use the getters below.
    Vec picture_size;  // Window size rotated by orientation setting
    UniqueArray<PageView> pages;
    Vec spread_size;  // unzoomed coordinates
    float zoom;
    Vec offset;  // zoomed coordinates

    bool need_picture_size = true;
    bool need_spread = true;  // both pages and spread_size
    bool need_zoom = true;
    bool need_offset = true;
    bool need_title = true;
    bool need_picture = true;

     // These model the dependency graph of view props
    void update_picture_size () { need_picture_size = true; update_zoom(); }
    void update_spread () { need_spread = true; update_zoom(); }
    void update_zoom () { need_zoom = true; update_offset(); }
    void update_offset () { need_offset = true; update_title(); update_picture(); }
    void update_title () { need_title = true; }
    void update_picture () { need_picture = true; }

     // Lazy getters.
    Vec get_picture_size ();
    Slice<PageView> get_pages ();
    Vec get_spread_size ();
    float get_zoom ();
    Vec get_offset ();

     // Restrict some properties based on current view
    float clamp_zoom (float);
    Vec clamp_offset (Vec);

     // change glViewport and update projection
    void window_size_changed (geo::IVec new_size);
     // Returns true if drawing was actually done.
    bool draw_if_needed ();
};

} // namespace liv
