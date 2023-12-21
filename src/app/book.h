// Implements a collection of images

#pragma once

#include "../dirt/uni/common.h"
#include "../dirt/wind/window.h"
#include "common.h"
#include "layout.h"
#include "page.h"
#include "page-block.h"
#include "settings.h"

struct SDL_Window;

namespace app {

struct Book {
    explicit Book (
        App& app,
         // All page filenames.
        Slice<AnyString> page_filenames,
         // Either folder or list filename.  Will be used as memory key.  This
         // should be an absolute filename.
        const AnyString& book_filename = "",
         // Page filename to start at (starts at page 1 if empty)
        const AnyString& start_filename = ""
    );
    ~Book ();

    const Settings* settings;
    Memory* memory;

    ///// Book contents
    PageBlock block;

    IRange viewing_pages;
    IRange visible_pages () const {
        return viewing_pages & block.valid_pages();
    }

    ///// Display parameters
    Fill window_background = BLACK;

     // TODO: combine these into something
    LayoutParams layout_params;
    PageParams page_params;

    ///// Controls
     // Takes a 1-based page offset.  viewing_pages will be {off - 1, off +
     // spread_count - 1}
     // Clamps to valid page offset (such that there is at least one page being
     // viewed)
    void set_page_offset (int32 off);
    int32 get_page_offset () const { return viewing_pages.l + 1; }

     // Set number of pages to view simultaneously.  Clamps to 1..2048
    void set_spread_count (int32 count);

     // Add to current page (stopping at first/last page)
    void seek (int32 count) {
        int64 off = get_page_offset() + count;
        set_page_offset(clamp(off, -2048, int32(GINF)));
    }
     // Increment current page(s) by spread_count
    void next () {
        seek(size(viewing_pages));
    }
    void prev () {
        seek(-size(viewing_pages));
    }

     // Set direction to display multiple pages
    void set_spread_direction (SpreadDirection);

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

    bool is_minimized () const;

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
