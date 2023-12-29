#pragma once

#include "../dirt/uni/common.h"
#include "../dirt/wind/window.h"
#include "common.h"
#include "layout.h"

namespace liv {

struct BookView {
    explicit BookView (Book* book);
    ~BookView ();

     // Object parent
    Book* book;

    wind::Window window;
     // Set these to nullopt or false when you change things they depend on.
    std::optional<Spread> spread;
    const Spread& get_spread ();
    std::optional<Layout> layout;
    const Layout& get_layout ();
     // Returns true if drawing was actually done.
    bool draw_if_needed ();
    bool need_draw = true;

    bool is_fullscreen () const;
    void set_fullscreen (bool);
    bool is_minimized () const;
    geo::IVec get_window_size () const;
    void window_size_changed (geo::IVec new_size);
};

} // namespace liv
