#include "commands.h"

#include <SDL2/SDL_video.h>
#include "../dirt/uni/io.h"
#include "app.h"
#include "book.h"
#include "settings.h"

namespace liv::commands {

///// APP COMMANDS

static void quit_ () {
    if (current_app) current_app->stop();
}
Command quit (quit_, "quit", "Quit application");

///// BOOK COMMANDS

static void next_ () {
    if (current_book) current_book->next();
}
Command next (next_, "next", "Go to next page or pages");

static void prev_ () {
    if (current_book) current_book->prev();
}
Command prev (prev_, "prev", "Go to previous page or pages");

static void seek_ (int32 count) {
    if (current_book) current_book->seek(count);
}
Command seek (seek_, "seek", "Add given amount to the current page number");

static void print_current_filename_ () {
    if (!current_book) return;
    auto visible = current_book->visible_pages();
    if (empty(visible)) return;
    auto page = current_book->block.get(visible.l);
    print_utf8(cat(page->filename, "\n"));
}
Command print_current_filename (print_current_filename_, "print_current_filename", "Print the filename of the first current page to stdout.");

///// LAYOUT COMMANDS

static void spread_count_ (int32 count) {
    if (current_book) current_book->set_spread_count(count);
}
Command spread_count (spread_count_, "spread_count", "Change number of pages to view at once");

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

static void zoom_multiply_ (float factor) {
    if (current_book) current_book->zoom_multiply(factor);
}
Command zoom_multiply (zoom_multiply_, "zoom_multiply", "Multiply zoom by a factor");

static void reset_layout_ () {
    if (current_book) current_book->reset_layout();
}
Command reset_layout (reset_layout_, "reset_layout", "Reset layout parameters to default");

///// PAGE COMMANDS

static void interpolation_mode_ (InterpolationMode mode) {
    if (current_book) {
        current_book->set_interpolation_mode(mode);
    }
}
Command interpolation_mode (interpolation_mode_, "interpolation_mode", "Set the pixel interpolation mode: nearest, linear, or cubic");

///// WINDOW COMMANDS

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

static void window_background_ (Fill bg) {
    if (current_book) current_book->set_window_background(bg);
}
Command window_background (window_background_, "window_background", "Change window background fill");

} // namespace liv::commands
