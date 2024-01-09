
#include "../dirt/iri/path.h"
#include "app.h"
#include "common.h"
#include "book-state.h"
#include "book-view.h"
#include "book-source.h"
#include "page-block.h"

namespace liv {

 // Collect all the different parts necessary to implement a book
struct Book {
    App* app;
    std::unique_ptr<BookSource> source;
    BookState state;
    BookView view;
    PageBlock block;
    bool need_memorize = false;

    explicit Book (
        App* app,
        std::unique_ptr<BookSource> src,
        std::unique_ptr<Settings> settings
    ) :
        app(app),
        source(move(src)),
        state(move(settings)),
        view(this),
        block(*state.settings, *source)
    { }
    ~Book () { }

     // TODO: move these to .cpp file

    IRange visible_range () {
        return state.viewing_range() & IRange{0, block.pages.size()};
    }

     // Commands
    void fullscreen () {
        view.set_fullscreen(!view.is_fullscreen());
        view.layout = {};
        view.need_draw = true;
    }

    void next () {
        seek(state.settings->get(&LayoutSettings::spread_count));
        need_memorize = true;
    }

    void prev () {
        seek(-state.settings->get(&LayoutSettings::spread_count));
        need_memorize = true;
    }

    void seek (int32 offset) {
        state.set_page_offset(state.page_offset + offset, block);
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void remove_current_page () {
        auto visible = visible_range();
        if (!size(visible)) return;
        block.unload_page(block.get(visible.l));
        block.pages.erase(visible.l);
         // Reclamp page offset
        state.set_page_offset(state.page_offset, block);
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void sort (SortMethod method) {
        auto visible = visible_range();
        IRI current_location = size(visible)
            ? block.pages[visible.l]->location
            : IRI();
        block.resort(method);
        if (current_location) {
            for (usize i = 0; i < size(block.pages); i++) {
                if (block.pages[i]->location == current_location) {
                    state.set_page_offset(i, block);
                    break;
                }
            }
        }
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void spread_count (int32 count) {
        state.set_spread_count(count, block);
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void spread_direction (Direction dir) {
        state.settings->layout.spread_direction = {dir};
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void auto_zoom_mode (AutoZoomMode mode) {
        state.set_auto_zoom_mode(mode);
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void align (Vec small, Vec large) {
        state.set_align(small, large);
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void zoom_multiply (float factor) {
        state.zoom_multiply(factor, view);
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void reset_layout () {
        state.reset_layout();
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void interpolation_mode (InterpolationMode mode) {
        state.settings->render.interpolation_mode = mode;
        view.need_draw = true;
        need_memorize = true;
    }

    void window_background (Fill bg) {
        state.settings->render.window_background = bg;
        view.need_draw = true;
        need_memorize = true;
    }

    void transparency_background (Fill bg) {
        state.settings->render.transparency_background = bg;
        view.need_draw = true;
        need_memorize = true;
    }

     // Not a command, but we need to figure out how to make this configurable.
    void drag (Vec amount) {
        state.drag(amount, view);
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }
};

} // namespace liv
