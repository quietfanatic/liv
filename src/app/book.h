// Implements a collection of images

#pragma once

#include "../base/uni/common.h"
#include "../base/wind/window.h"
#include "common.h"
#include "layout.h"
#include "page.h"
#include "pages.h"
#include "settings.h"

struct SDL_Window;

namespace app {

struct Book {
    const Settings* settings;

    ///// Book contents
    Pages pages;
     // 1-based index.  The page nos currently being viewed are
     // page_offset .. (page_offset + spread_pages - 1).  Valid values are such
     // that there is at least one valid page being viewed:
     // (1 - (spread_pages-1)) .. (pages.size() + (spread_pages-1))
    isize page_offset = 1;
     // Number of pages currently being viewed.
    isize spread_pages = 1;

    isize first_visible_page () const {
        return pages.first_visible_page({page_offset, spread_pages});
    }
    isize last_visible_page () const {
        return pages.last_visible_page({page_offset, spread_pages});
    }

    explicit Book (
        App& app,
        FilesToOpen&& files
    );
    ~Book ();

    ///// Display parameters
    Fill window_background = BLACK;

    LayoutParams layout_params;
    PageParams page_params;

    ///// Controls
     // Clamps to valid page offset
    void set_page_offset (isize off);
     // Set number of pages to view simultaneously.
     // clamps to 1..settings.max_spread_pages
    void set_spread_pages (isize count);
     // Set direction to display multiple pages
    void set_spread_direction (SpreadDirection);

     // Increment current page(s) by spread_pages
    void next () { set_page_offset(page_offset + spread_pages); }
    void prev () { set_page_offset(page_offset - spread_pages); }
     // Add to current page (stopping at first/last page)
    void seek (isize count) { set_page_offset(page_offset + count); }

    void set_window_background (Fill);
    void set_auto_zoom_mode (AutoZoomMode);
    void set_align (geo::Vec small, geo::Vec large);
    void set_interpolation_mode (InterpolationMode);
     // Adds amount to view.offset
    void drag (geo::Vec amount);

    void zoom_multiply (float factor);

     // Reset all layout parameters
    void reset_layout ();

    bool is_fullscreen () const;
    void set_fullscreen (bool);

    ///// Internal stuff
    wind::Window window;
     // Set these to nullopt or false when you change things they depend on.
    std::optional<Spread> spread;
    const Spread& get_spread ();
    std::optional<Layout> layout;
    const Layout& get_layout ();
     // Returns true if drawing was actually done.
    bool draw_if_needed ();
    bool need_draw = true;

     // Preload images perhaps
     // Returns true if any processing was actually done
    bool idle_processing ();

    geo::IVec get_window_size () const;

    void window_size_changed (geo::IVec new_size);
};

} // namespace app
