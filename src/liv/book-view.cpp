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
        book->state.settings->get(&WindowSettings::size)
    )
{
    plog("created window");
    SDL_SetWindowResizable(window, SDL_TRUE);
    expect(!SDL_GL_SetSwapInterval(1));
    if (book->state.settings->get(&WindowSettings::fullscreen)) {
        window.set_fullscreen(true);
    }
    plog("set window props");
    glow::init();
    plog("fetched gl functions");
    if (!book->state.settings->get(&WindowSettings::hidden)) {
        SDL_ShowWindow(window);
    }
    plog("showed window");
}

BookView::~BookView () { }

const Spread& BookView::get_spread () {
    if (need_spread) {
        spread = Spread(*book);
        need_spread = false;
        need_projection = true;
    }
    return spread;
}

const Projection& BookView::get_projection () {
    get_spread();
    if (need_projection) {
        projection = Projection(book->state, spread, window.size());
        need_projection = false;
        need_title = true;
        need_picture = true;
    }
    return projection;
}

bool BookView::draw_if_needed () {
    if (!need_something) return false;
    get_projection();
    if (need_title) {
         // Theoretically we track whether we need to do the title independently
         // of whether we need to draw.
        AnyString title;
        IRange visible = book->visible_range();
        if (book->block.count() == 0) {
            title = "Little Image Viewer (nothing loaded)";
        }
        else if (empty(visible)) {
            title = "Little Image Viewer (no pages visible)";
        }
        else {
            auto& title_format = book->state.settings->get(&WindowSettings::title);
            UniqueString t;
            title_format.write(t, book);
            title = t;
        }
         // This might be an X-specific problem, but if SDL_SetWindowTitle is given
         // invalid Unicode, the window title doesn't get updated.  There's no way
         // to check that this happened, because the string returned by
         // SDL_GetWindowTitle is the requested title, not the string that's
         // currently being rendered on the title bar.  Checking the validity of the
         // Unicode ahead of time would require having access to a table of hundreds
         // of thousands of characters.  So the only thing we can really do is to
         // set the error message title, then set the desired title, and if the
         // desired title has invalid unicode, the old error title will remain
         // rendered.
        SDL_SetWindowTitle(window, "Little Image Viewer (invalid unicode in title)");
        SDL_SetWindowTitle(window, title.c_str());
        need_title = false;
    }
    if (need_picture) {
         // TODO: Currently we have a different context for each window, would it
         // be better to share a context between all windows?  Not that we currently
         // allow multiple windows per process.
        SDL_GL_MakeCurrent(window, window.gl_context);
         // Draw background
        auto bg = book->state.settings->get(&RenderSettings::window_background);
        glClearColor(
            bg.r / 255.f,
            bg.g / 255.f,
            bg.b / 255.f,
            bg.a / 255.f // Alpha is probably ignored
        );
        glClear(GL_COLOR_BUFFER_BIT);
         // Draw spread
        for (auto& spread_page : spread.pages) {
            spread_page.page->last_viewed_at = uni::now();
            Rect spread_rect = Rect(
                spread_page.offset,
                spread_page.offset + spread_page.page->size
            );
            Rect window_rect = spread_rect * projection.zoom + projection.offset;
             // Convert to OpenGL coords (-1,-1)..(+1,+1)
            Rect screen_rect = window_rect / projection.size * float(2) - Vec(1, 1);
             // Draw
            spread_page.page->draw(*book->state.settings, projection.zoom, screen_rect);
        }
        plog("drew view");
         // vsync
        SDL_GL_SwapWindow(window);
        plog("swapped window");
        need_picture = false;
    }
    need_something = false;
    expect(!need_spread && !need_projection);
    return true;
}

void BookView::window_size_changed (IVec size) {
     // TODO: write window.size setting
    require(size.x > 0 && size.y > 0);
    glViewport(0, 0, size.x, size.y);
    update_projection();
}

} // namespace liv
