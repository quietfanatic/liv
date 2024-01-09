
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
     // TODO: is this still necessary?
    App* app;
     // Data roughly flows downward
    std::unique_ptr<BookSource> source;
    PageBlock block;
    BookState state;
    BookView view;
    bool need_memorize = false;

    explicit Book (
        App* app,
        std::unique_ptr<BookSource> src,
        BookState st
    ) :
        app(app),
        source(move(src)),
        block(*st.settings, *source),
        state(move(st)),
        view(this)
    { }
    ~Book () { }

    IRange visible_range () {
        return state.viewing_range() & IRange{0, block.pages.size()};
    }

     // Commands
    void fullscreen ();
    void set_page_offset (int32);
    void next ();
    void prev ();
    void seek (int32);
    void remove_current_page ();
    void sort (SortMethod);
    void spread_count (int32);
    void spread_direction (Direction);
    void auto_zoom_mode (AutoZoomMode);
    void align (Vec small, Vec large);
    void zoom_multiply (float);
    void reset_layout ();
    void interpolation_mode (InterpolationMode);
    void window_background (Fill);
    void transparency_background (Fill);
     // Not a command, but we need to figure out how to make this configurable.
    void drag (Vec amount);
};

} // namespace liv
