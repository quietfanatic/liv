#pragma once

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
        std::unique_ptr<BookSource>&& src
    ) :
        app(app),
        source(move(src)),
        state(this),
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
        int64 no = state.get_page_number() + offset;
        state.set_page_number(clamp(no, -2048, int32(GINF)));
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    AnyString current_filename () {
        auto visible = state.visible_range();
        if (empty(visible)) return "";
        auto page = block.get(visible.l);
        return iri::to_fs_path(page->location);
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
        state.layout_params = LayoutParams(app->settings);
        view.spread = {};
        view.layout = {};
        view.need_draw = true;
        need_memorize = true;
    }

    void interpolation_mode (InterpolationMode mode) {
        state.render_params.interpolation_mode = mode;
        view.need_draw = true;
        need_memorize = true;
    }

    void window_background (Fill bg) {
        state.render_params.window_background = bg;
        view.need_draw = true;
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
