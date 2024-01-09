#include <algorithm>
#include <SDL2/SDL_clipboard.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_messagebox.h>
#include "../dirt/control/command.h"
#include "../dirt/uni/io.h"
#include "../dirt/uni/shell.h"
#include "../dirt/uni/text.h"
#include "app.h"
#include "book.h"
#include "list.h"
#include "settings.h"

namespace liv::commands {
using namespace control;

///// APP AND WINDOW COMMANDS

static void quit_ () {
    if (current_app) current_app->stop();
}
Command quit (quit_, "quit", "Quit application");

static void fullscreen_ () {
    if (current_book) current_book->fullscreen();
}
Command fullscreen (fullscreen_, "fullscreen", "Toggle fullscreen mode");

static void leave_fullscreen_ () {
    if (current_book && current_book->view.is_fullscreen()) {
        current_book->fullscreen();
    }
}
Command leave_fullscreen (leave_fullscreen_, "leave_fullscreen", "Leave fullscreen mode");

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

static void prompt_command_ () {
    auto res = run({
        "zenity", "--entry", cat("--title=Input command"),
        cat("--text=See commands.h for available commands"),
    });
    if (res.ret != 0) {
        if (res.command_not_found()) {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_ERROR,
                "Cannot run zenity",
                "This action is only available if zenity is installed.",
                current_book->view.window
            );
        }
        return;
    }
    try {
        Statement cmd;
        ayu::item_from_list_string(&cmd, res.out);
        cmd();
    }
    catch (std::exception& e) {
        run({
            "zenity", "--error", "--title=Command failed",
            cat("--text=", "This command: ", res.out,
                "\nthrew an exception: ", e.what()
            )
        });
    }
}
Command prompt_command (prompt_command_, "prompt_command", "Prompt for a command with a dialog box");

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
        auto res = run({
            "zenity", cat("--title=", t), "--info", cat("--text=", m)
        });
        if (res.command_not_found()) {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                t.c_str(), m.c_str(),
                current_book->view.window
            );
        }
    }
}
Command message_box (message_box_, "message_box", "Show a message box with formatted title and content");

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

static void add_to_list_ (const AnyString& list, SortMethod sort) {
    if (!current_book) return;
    auto visible = current_book->visible_range();
    if (!size(visible)) return;

    auto loc = iri::from_fs_path(list);
     // Read
    UniqueArray<IRI> entries;
    try {
        entries = read_list(loc);
    }
    catch (Error& e) {
        if (e.code == e_OpenFailed) {
             // New file, create it implicitly
        }
        else throw;
    }
     // Add and sort
    entries.push_back(
        current_book->block.pages[visible.l]->location
    );
    sort_iris(entries.begin(), entries.end(), sort);
     // Remove duplicates
     // TODO: don't if [unsorted]
    auto new_end = std::unique(entries.begin(), entries.end());
    entries.resize(new_end - entries.begin());
     // Write
    write_list(loc, entries);
}
Command add_to_list (add_to_list_, "add_to_list", "Add current page filename to a list file and sort it");

static void remove_from_list_ (const AnyString& list) {
    if (!current_book) return;
    auto visible = current_book->visible_range();
    if (!size(visible)) return;
    auto loc = iri::from_fs_path(list);
    auto entries = read_list(loc);
    auto new_end = std::remove(
        entries.begin(), entries.end(),
        current_book->block.pages[visible.l]->location
    );
    entries.impl.size = new_end - entries.impl.data;
    write_list(loc, entries);
}
Command remove_from_list (remove_from_list_, "remove_from_list", "Remove current page from list file");

static void clipboard_text_ (const FormatList& fmt) {
    if (!current_book) return;
    UniqueString text;
    fmt.write(text, current_book);
    SDL_SetClipboardText(text.c_str());
}
Command clipboard_text (clipboard_text_, "clipboard_text", "Set clipboard text with format list");

static void remove_from_book_ () {
    if (!current_book) return;
    current_book->remove_current_page();
}
Command remove_from_book (remove_from_book_, "remove_from_book", "Remove current page from current book");

static void move_to_folder_ (const AnyString& folder) {
    if (!current_book) return;
    auto visible = current_book->visible_range();
    if (!size(visible)) return;
    auto& loc = current_book->block.pages[visible.l]->location;
    auto new_path = cat(folder, '/', iri::path_filename(loc.path()));
    fs::rename(iri::to_fs_path(loc), new_path);
}
Command move_to_folder (move_to_folder_, "move_to_folder", "Move current page to a folder");

static void sort_ (SortMethod method) {
    if (!current_book) return;
    current_book->sort(method);
}
Command sort (sort_, "sort", "Change sort method of current book");

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
