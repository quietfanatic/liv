#include "book-view.h"

#include <SDL2/SDL_video.h>
#include "../dirt/uni/time.h"
#include "book.h"
#include "layout.h"

namespace liv {

BookView::BookView (Book* book) :
    book(book),
    window(
        "Little Image Viewer",
        book->app->settings->get(&WindowSettings::size)
    )
{
    SDL_SetWindowResizable(window, SDL_TRUE);
    expect(!SDL_GL_SetSwapInterval(1));
    if (book->app->settings->get(&WindowSettings::fullscreen)) {
        set_fullscreen(true);
    }
    glow::init();
    if (!book->app->hidden) SDL_ShowWindow(window);
}

BookView::~BookView () { }

const Spread& BookView::get_spread () {
    if (!spread) {
        spread.emplace(
            book->block, book->state.spread_range, book->state.layout_params
        );
    }
    return *spread;
}

const Layout& BookView::get_layout () {
    if (!layout) {
        layout.emplace(
            book->app->settings, get_spread(),
            book->state.layout_params, get_window_size()
        );
    }
    return *layout;
}

bool BookView::draw_if_needed () {
    if (!need_draw) return false;
    need_draw = false;
    auto& spread = get_spread();
    auto& layout = get_layout();
     // TODO: Currently we have a different context for each window, would it
     // be better to share a context between all windows?
    SDL_GL_MakeCurrent(window, window.gl_context);
     // Draw background
    auto bg = book->state.render_params.window_background;
    glClearColor(
        bg.r / 255.f,
        bg.g / 255.f,
        bg.b / 255.f,
        bg.a / 255.f // Alpha is probably ignored
    );
    glClear(GL_COLOR_BUFFER_BIT);
     // Draw spread
    Vec window_size = get_window_size();
    for (auto& spread_page : spread.pages) {
        spread_page.page->last_viewed_at = uni::now();
        Rect spread_rect = Rect(
            spread_page.offset,
            spread_page.offset + spread_page.page->size
        );
        Rect window_rect = spread_rect * layout.zoom + layout.offset;
         // Convert to OpenGL coords (-1,-1)..(+1,+1)
        Rect screen_rect = window_rect / window_size * float(2) - Vec(1, 1);
         // Draw
        spread_page.page->draw(book->state.render_params, layout.zoom, screen_rect);
    }
     // Generate title
    AnyString title;
    IRange visible = book->state.visible_range();
    if (book->block.count() == 0) {
        title = "Little Image Viewer (nothing loaded)";
    }
    else if (empty(visible)) {
        title = "Little Image Viewer (no pages visible)";
    }
    else {
        UniqueString t;
        book->app->settings->get(&WindowSettings::title).write(t, book);
        title = t;
    }
    SDL_SetWindowTitle(window, title.c_str());
     // vsync
    SDL_GL_SwapWindow(window);
    return true;
}

bool BookView::is_fullscreen () const {
    auto flags = glow::require_sdl(SDL_GetWindowFlags(window));
    return flags & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN);
}

void BookView::set_fullscreen (bool fs) {
     // This will trigger a window_size_changed, so no need to clear layout or
     // set need_draw
    glow::require_sdl(!SDL_SetWindowFullscreen(
        window,
        fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
    ));
}

bool BookView::is_minimized () const {
    auto flags = glow::require_sdl(SDL_GetWindowFlags(window));
    return flags & SDL_WINDOW_MINIMIZED;
}

IVec BookView::get_window_size () const {
    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    require(w > 0 && h > 0);
    return {w, h};
}

void BookView::window_size_changed (IVec size) {
    require(size.x > 0 && size.y > 0);
    glViewport(0, 0, size.x, size.y);
    layout = {};
    need_draw = true;
}

} // namespace liv
