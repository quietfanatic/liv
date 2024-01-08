
#include "../dirt/iri/path.h"
#include "app.h"
#include "common.h"
#include "book-state.h"
#include "book-view.h"
#include "book-source.h"
#include "page-block.h"

namespace liv {

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
        state(this, move(settings)),
        view(this),
        block(&*source)
    { }
    ~Book () { }

     // Commands
    void fullscreen () {
        view.set_fullscreen(!view.is_fullscreen());
        view.layout = {};
        view.need_draw = true;
    }

    void next () {
        seek(size(state.spread_range));
        need_memorize = true;
    }

    void prev () {
        seek(-size(state.spread_range));
        need_memorize = true;
    }

    void seek (int32 offset) {
        int32 clamped = clamp(state.spread_range.l + offset, -2048, GINF);
        state.set_page_offset(clamped);
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void remove_current_page () {
        auto visible = state.visible_range();
        if (!size(visible)) return;
        block.unload_page(block.get(visible.l));
        block.pages.erase(visible.l);
        source->pages.erase(visible.l);
         // This will clamp the value so that there's still at least one visible
         // page.
        state.set_page_offset(visible.l);
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void sort (SortMethod method) {
        auto visible = state.visible_range();
        IRI current_location = size(visible)
            ? source->pages[visible.l]
            : IRI();
        source->change_sort_method(method);
        block.source_updated(&*source);
        if (current_location) {
            int32 new_offset = source->find_page_offset(current_location);
            state.set_page_offset(new_offset);
        }
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void spread_count (int32 count) {
        state.set_spread_count(count);
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void spread_direction (Direction dir) {
        state.layout_params.spread_direction = dir;
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
        state.zoom_multiply(factor);
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void reset_layout () {
        state.layout_params = LayoutParams(*state.settings);
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void interpolation_mode (InterpolationMode mode) {
        state.settings->RenderSettings::interpolation_mode = mode;
        view.need_draw = true;
        need_memorize = true;
    }

    void window_background (Fill bg) {
        state.settings->RenderSettings::window_background = bg;
        view.need_draw = true;
        need_memorize = true;
    }

    void transparency_background (Fill bg) {
        state.settings->RenderSettings::transparency_background = bg;
        view.need_draw = true;
        need_memorize = true;
    }

     // Not a command, but we need to figure out how to make this configurable.
    void drag (Vec amount) {
        state.drag(amount);
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }
};

} // namespace liv
