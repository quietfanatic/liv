#pragma once

#include "../dirt/iri/path.h"
#include "app.h"
#include "common.h"
#include "book-source.h"
#include "book-state.h"
#include "book-view.h"
#include "page-block.h"

namespace liv {

 // This collects all the different parts needed to manage a book, and fills the
 // role of a controller.
struct Book {
     // Data roughly flows downward
    BookSource source;
    PageBlock block;
    BookState state;
    BookView view;

     // To work around a bug where gaining focus from another window closing due
     // to a keystroke makes our window receive the keystroke that closed the
     // other window.
    u32 last_focused = 0;

    bool need_mark = false;

    explicit Book (
        BookSource&&, PageBlock&&, BookState&&
    );
    explicit Book (
        BookSource&&, std::unique_ptr<Settings>
    );

    ~Book () { }

    IRange visible_range () {
        return state.viewing_range() & IRange{0, block.pages.size()};
    }

    void on_event (SDL_Event*);

     // Commands
    void fullscreen ();
    void set_page_offset (i32);
    void next ();
    void prev ();
    void seek (i32);
    void go_next (Direction);
    void go (Direction, i32);
    void remove_current_page ();
    void sort (SortMethod);
    void spread_count (i32);
    void spread_direction (Direction);
    void auto_zoom_mode (AutoZoomMode);
    void align (Vec small, Vec large);
    void zoom_multiply (float);
    void reset_layout ();
    void reset_settings ();
    void interpolation_mode (InterpolationMode);
    void window_background (Fill);
    void transparency_background (Fill);
     // Not a command, but we need to figure out how to make this configurable.
    void drag (Vec amount);
};

} // namespace liv
