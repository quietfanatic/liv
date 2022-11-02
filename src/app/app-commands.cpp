#include "app-commands.h"

#include <SDL2/SDL_video.h>
#include "app.h"
#include "book.h"
#include "settings.h"

using namespace geo;

namespace app::command {

static void next_ () {
    if (current_book) current_book->next();
}
Command next (next_, "next", "Go to next page or pages");

static void prev_ () {
    if (current_book) current_book->prev();
}
Command prev (prev_, "prev", "Go to previous page or pages");

static void seek_ (isize count) {
    if (current_book) current_book->seek(count);
}
Command seek (seek_, "seek", "Add given amount to the current page number");

static void auto_zoom_mode_ (AutoZoomMode mode) {
    if (current_book) {
        current_book->set_auto_zoom_mode(mode);
    }
}
Command auto_zoom_mode (auto_zoom_mode_, "auto_zoom_mode", "Set auto zoom mode: fit or original");

static void align_ (Vec small, Vec large) {
    if (current_book) {
        current_book->set_align(small, large);
    }
}
Command align (align_, "align", "Set page alignment (small_align and large_align)");

static void interpolation_mode_ (InterpolationMode mode) {
    if (current_book) {
        current_book->set_interpolation_mode(mode);
    }
}
Command interpolation_mode (interpolation_mode_, "interpolation_mode", "Set the pixel interpolation mode: nearest, linear, or cubic");

static void zoom_multiply_ (float factor) {
    if (current_book) current_book->zoom_multiply(factor);
}
Command zoom_multiply (zoom_multiply_, "zoom_multiply", "Multiply zoom by a factor");

 // TODO: move logic to Book
static void fullscreen_ () {
    if (current_book) {
        current_book->set_fullscreen(!current_book->is_fullscreen());
    }
}
Command fullscreen (fullscreen_, "fullscreen", "Toggle fullscreen mode");

static void leave_fullscreen_or_quit_ () {
    if (current_book && current_book->is_fullscreen()) {
        current_book->set_fullscreen(false);
    }
    else if (current_app) {
        current_app->stop();
    }
}
Command leave_fullscreen_or_quit (
    leave_fullscreen_or_quit_, "leave_fullscreen_or_quit", "Leave fullscreen mode, or quit app if not in fullscreen mode"
);

static void quit_ () {
    if (current_app) current_app->stop();
}
Command quit (quit_, "quit", "Quit application");

} // namespace app::command
