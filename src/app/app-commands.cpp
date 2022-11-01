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
        current_book->set_fit_mode(mode);
    }
}
Command fit_mode (fit_mode_, "fit_mode", "Set fit mode: fit, stretch, or manual");

 // TODO: move logic to Book
static void fullscreen_ () {
    if (current_book) {
        current_book->set_fullscreen(!current_book->view.fullscreen);
    }
}
Command fullscreen (fullscreen_, "fullscreen", "Toggle fullscreen mode");

static void leave_fullscreen_or_quit_ () {
     // TODO: For some reason when leaving fullscreen on pressing ESC, the app
     // receives another ESC input, causing it to quit immediately.  Add a
     // timeout here so that doesn't happen, or something?
    if (current_book && current_book->view.fullscreen) {
        current_book->set_fullscreen(false);
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
