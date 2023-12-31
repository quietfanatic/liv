#include "commands.h"

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_messagebox.h>
#include "../dirt/uni/io.h"
#include "../dirt/uni/text.h"
#include "app.h"
#include "book.h"
#include "settings.h"

namespace liv::commands {

///// APP AND WINDOW COMMANDS

static void quit_ () {
    if (current_app) current_app->stop();
}
Command quit (quit_, "quit", "Quit application");

static void fullscreen_ () {
    if (current_book) current_book->fullscreen();
}
Command fullscreen (fullscreen_, "fullscreen", "Toggle fullscreen mode");

static void leave_fullscreen_or_quit_ () {
    if (current_book && current_book->view.is_fullscreen()) {
        current_book->fullscreen();
    }
    else if (current_app) {
        current_app->stop();
    }
}
Command leave_fullscreen_or_quit (
    leave_fullscreen_or_quit_, "leave_fullscreen_or_quit", "Leave fullscreen mode, or quit app if not in fullscreen mode"
);

///// BOOK AND PAGE COMMANDS

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

static void say_ (const FormatList& fmt) {
    if (current_book) {
        UniqueString s;
        fmt.write(s, current_book);
        print_utf8(cat(move(s), "\n"));
    }
}
Command say (say_, "say", "Print a formatted string to stdout with a newline.");

static void message_box_ (const FormatList& title, const FormatList& message) {
    if (current_book) {
        UniqueString t;
        title.write(t, current_book);
        UniqueString m;
        message.write(m, current_book);
        static bool have_zenity = system("zenity --help > /dev/null") == 0;
        if (have_zenity) {
            if (system(cat(
                "zenity --title='", escape_for_shell(t), "' ",
                "--info --text='", escape_for_shell(m), "'\0"
            ).data()) != 0) { }
        }
        else {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                t.c_str(), m.c_str(),
                current_book->view.window
            );
        }
    }
}
Command message_box (message_box_, "message_box", "Show a message box with formatted title and content");

///// LAYOUT COMMANDS

static void spread_count_ (int32 count) {
    if (current_book) current_book->spread_count(count);
}
Command spread_count (spread_count_, "spread_count", "Change number of pages to view at once");

static void auto_zoom_mode_ (AutoZoomMode mode) {
    if (current_book) current_book->auto_zoom_mode(mode);
}
Command auto_zoom_mode (auto_zoom_mode_, "auto_zoom_mode", "Set auto zoom mode: fit or original");

static void align_ (Vec small, Vec large) {
    if (current_book) current_book->align(small, large);
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

///// RENDER COMMANDS

static void interpolation_mode_ (InterpolationMode mode) {
    if (current_book) current_book->interpolation_mode(mode);
}
Command interpolation_mode (interpolation_mode_, "interpolation_mode", "Set the pixel interpolation mode: nearest, linear, or cubic");

static void window_background_ (Fill bg) {
    if (current_book) current_book->window_background(bg);
}
Command window_background (window_background_, "window_background", "Change window background fill");

static void transparency_background_ (Fill bg) {
    if (current_book) current_book->transparency_background(bg);
}
Command transparency_background (transparency_background_, "transparency_background", "Change fill behind transparent images");

} // namespace liv::commands
