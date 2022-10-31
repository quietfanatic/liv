#include "app-commands.h"

#include <SDL2/SDL_video.h>
#include "app.h"
#include "book.h"
#include "settings.h"

namespace app::command {

static void next_ () {
    if (current_book) current_book->next();
}
Command next (next_, "next", "Go to next page or pages");

static void prev_ () {
    if (current_book) current_book->prev();
}
Command prev (prev_, "prev", "Go to previous page or pages");

static void quit_ () {
    if (current_app) current_app->stop();
}
Command quit (quit_, "quit", "Quit application");

static void fit_mode_ (FitMode mode) {
    if (current_book) {
        current_book->view.fit_mode = mode;
        current_book->draw();
    }
}
Command fit_mode (fit_mode_, "fit_mode", "Set fit mode: fit, stretch, or manual");

 // TODO: move logic to Book
static void fullscreen_ () {
    if (current_book) {
        bool& fs = current_book->view.fullscreen;
        fs = !fs;
        AS(!SDL_SetWindowFullscreen(
            current_book->window.sdl_window,
            fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
        ));
        current_book->draw();
    }
}
Command fullscreen (fullscreen_, "fullscreen", "Toggle fullscreen mode");

static void leave_fullscreen_or_quit_ () {
    if (current_book && current_book->view.fullscreen) {
        current_book->view.fullscreen = false;
        AS(!SDL_SetWindowFullscreen(current_book->window.sdl_window, 0));
        current_book->draw();
    }
    else if (current_app) {
        current_app->stop();
    }
}
Command leave_fullscreen_or_quit (
    leave_fullscreen_or_quit_, "leave_fullscreen_or_quit", "Leave fullscreen mode, or quit app if not in fullscreen mode"
);

static void zoom_multiply_ (float factor) {
    if (current_book) current_book->zoom_multiply(factor);
}
Command zoom_multiply (zoom_multiply_, "zoom_multiply", "Multiply zoom by a factor");

} // namespace app::command
