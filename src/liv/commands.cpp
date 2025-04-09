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
#include "mark.h"
#include "settings.h"

namespace liv::commands {
using namespace control;

///// APP AND WINDOW COMMANDS

static void quit () {
    if (current_app) current_app->stop();
}
CONTROL_COMMAND(quit, 0, "Quit application")

static void fullscreen () {
    if (current_book) {
        current_book->view.window.set_fullscreen(
            !current_book->view.window.is_fullscreen()
        );
    }
}
CONTROL_COMMAND(fullscreen, 0, "Toggle fullscreen mode")

static void leave_fullscreen () {
     // Check if we're already fullscreen to avoid generating a size changed
     // event.
    if (current_book && current_book->view.window.is_fullscreen()) {
        current_book->view.window.set_fullscreen(false);
    }
}
CONTROL_COMMAND(leave_fullscreen, 0, "Leave fullscreen mode")

static void leave_fullscreen_or_quit () {
    if (current_book && current_book->view.window.is_fullscreen()) {
        current_book->view.window.set_fullscreen(false);
    }
    else if (current_app) {
        current_app->stop();
    }
}
CONTROL_COMMAND(leave_fullscreen_or_quit, 0,
    "Leave fullscreen mode, or quit app if not in fullscreen mode"
)

static void prompt_command () {
    if (!current_book) return;
    auto last = current_book->state.settings->get(
        &WindowSettings::last_prompt_command
    );

    auto res = run({
        "zenity", "--entry", cat("--title=Input command"),
        "--text=See commands.h for available commands",
        cat("--entry-text=", last)
    });
    if (res.ret != 0) {
        if (res.command_wasnt_found()) {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_ERROR,
                "Cannot run zenity",
                "This action is only available if zenity is installed.",
                current_book->view.window
            );
        }
        return;
    }
    AnyString text = move(res.out);
    if (text && text.back() == '\n') text.pop_back();
    current_book->state.settings->window.last_prompt_command = text;
    current_book->need_mark = true;
    try {
        Statement cmd;
        ayu::item_from_list_string(&cmd, text);
        cmd();
    }
    catch (std::exception& e) {
        run({
            "zenity", "--error", "--title=Command failed", "--no-markup",
            cat("--text=", "This command: ", text,
                "\nthrew an exception: ", e.what()
            )
        });
    }
}
CONTROL_COMMAND(prompt_command, 0, "Prompt for a command with a dialog box")

static void say (const FormatList& fmt) {
    if (current_book) {
        UniqueString s;
        fmt.write(s, current_book);
        print_utf8(cat(move(s), "\n"));
    }
}
CONTROL_COMMAND(say, 1, "Print a formatted string to stdout with a newline.")

 // TODO: allow single parameter
static void message_box (const FormatList& title, const FormatList& message) {
    if (current_book) {
        UniqueString t;
        title.write(t, current_book);
        UniqueString m;
        message.write(m, current_book);
        auto res = run({
            "zenity", "--no-markup", cat("--title=", t), "--info", cat("--text=", m)
        });
        if (res.command_wasnt_found()) {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                t.c_str(), m.c_str(),
                current_book->view.window
            );
        }
    }
}
CONTROL_COMMAND(message_box, 2, "Show a message box with formatted title and content")

static void clipboard_text (const FormatList& fmt) {
    if (!current_book) return;
    UniqueString text;
    fmt.write(text, current_book);
    SDL_SetClipboardText(text.c_str());
}
CONTROL_COMMAND(clipboard_text, 1, "Set clipboard text with format list")

static void shell (const FormatList& fmt) {
    if (!current_book) return;
    UniqueString cmd;
    fmt.write(cmd, current_book);
    shell(cmd.c_str());
}
CONTROL_COMMAND(shell, 1, "Create a system shell command with a format list and run it.")

 // Not AnyArray because FormatList is not copyable
static void run (const UniqueArray<FormatList>& fmts) {
    if (!current_book) return;
    auto args = UniqueArray<UniqueString>(
        fmts.size(), [&fmts](u32 i)
    {
        UniqueString s;
        fmts[i].write(s, current_book);
        return s;
    });
    auto strs = UniqueArray<Str>(args.size(), [&args](u32 i){
        return Str(args[i]);
    });
    run(strs);
}
CONTROL_COMMAND(run, 1, "Run a system command with the command name and each argument from format lists.")

///// ACTION COMMANDS

static void next () {
    if (current_book) current_book->next();
}
CONTROL_COMMAND(next, 0, "Go to next page or pages")

static void prev () {
    if (current_book) current_book->prev();
}
CONTROL_COMMAND(prev, 0, "Go to previous page or pages")

static void seek (int32 count) {
    if (current_book) current_book->seek(count);
}
CONTROL_COMMAND(seek, 1, "Add given amount to the current page number")

static void go_next (Direction dir) {
    if (current_book) current_book->go_next(dir);
}
CONTROL_COMMAND(go_next, 1, "Move one spread count in the given direction")

static void go (Direction dir, int32 count) {
    if (current_book) current_book->go(dir, count);
}
CONTROL_COMMAND(go, 2, "Move in the given direction by the given number of pages")

static void trap_pointer (bool trap) {
    if (current_book) current_book->trap_pointer(trap);
}
CONTROL_COMMAND(trap_pointer, 1, "Set pointer trap mode")

///// LAYOUT COMMANDS

static void spread_count (int32 count) {
    if (current_book) current_book->spread_count(count);
}
CONTROL_COMMAND(spread_count, 1, "Change number of pages to view at once")

static void spread_direction (Direction dir) {
    if (current_book) current_book->spread_direction(dir);
}
CONTROL_COMMAND(spread_direction, 1, "Change direction to read book in")

static void auto_zoom_mode (AutoZoomMode mode) {
    if (current_book) current_book->auto_zoom_mode(mode);
}
CONTROL_COMMAND(auto_zoom_mode, 1, "Set auto zoom mode: fit or original")

static void set_zoom (float zoom) {
    if (current_book) current_book->set_zoom(zoom);
}
CONTROL_COMMAND(set_zoom, 1, "Set zoom to a specific amount")

static void zoom (float factor) {
    if (current_book) current_book->zoom(factor);
}
CONTROL_COMMAND(zoom, 1, "Multiply zoom by a factor")

static void align (Vec small, Vec large) {
    if (current_book) current_book->align(small, large);
}
CONTROL_COMMAND(align, 2, "Set page alignment (small_align and large_align)")

static void orientation (Direction o) {
    if (current_book) current_book->orientation(o);
}
CONTROL_COMMAND(orientation, 1, "Set page orientation")

static void reset_layout () {
    if (current_book) current_book->reset_layout();
}
CONTROL_COMMAND(reset_layout, 0, "Reset most layout parameters to default")

static void reset_settings () {
    if (current_book) current_book->reset_settings();
}
CONTROL_COMMAND(reset_settings, 0, "Reset all temporary settings to default")

///// RENDER COMMANDS

static void upscaler (Upscaler mode) {
    if (current_book) current_book->upscaler(mode);
}
CONTROL_COMMAND(upscaler, 1, "Set the upscaling interpolation mode")

static void deringer (Deringer mode) {
    if (current_book) current_book->deringer(mode);
}
CONTROL_COMMAND(deringer, 1, "Set upscale deringing mode")

static void downscaler (Downscaler mode) {
    if (current_book) current_book->downscaler(mode);
}
CONTROL_COMMAND(downscaler, 1, "Set the downscaling interpolation mode")

static void window_background (Fill bg) {
    if (current_book) current_book->window_background(bg);
}
CONTROL_COMMAND(window_background, 1, "Change window background fill")

static void transparency_background (Fill bg) {
    if (current_book) current_book->transparency_background(bg);
}
CONTROL_COMMAND(transparency_background, 1, "Change fill behind transparent images")

static void color_range (const ColorRange& range) {
    if (current_book) current_book->color_range(range);
}
CONTROL_COMMAND(color_range, 1, "Adjust the color output range with [[rl rh] [gl gh] [bl bh]]")

///// BOOK COMMANDS

static void sort (SortMethod method) {
    if (!current_book) return;
    current_book->sort(method);
}
CONTROL_COMMAND(sort, 1, "Change sort method of current book")

 // TODO: optional argument?
static void add_to_list (const AnyString& list, SortMethod sort) {
    if (!current_book) return;
    auto visible = current_book->visible_range();
    if (!size(visible)) return;

    auto loc = iri::from_fs_path(list);
    const IRI& entry = current_book->block.pages[visible.l]->location;
    add_to_list(loc, entry, sort);
}
CONTROL_COMMAND(add_to_list, 2, "Add current page filename to a list file and sort it")

static void remove_from_list (const AnyString& list) {
    if (!current_book) return;
    auto visible = current_book->visible_range();
    if (!size(visible)) return;
    auto loc = iri::from_fs_path(list);
    const IRI& entry = current_book->block.pages[visible.l]->location;
    liv::remove_from_list(loc, entry);
}
CONTROL_COMMAND(remove_from_list, 1, "Remove current page from list file")

static void remove_from_book () {
    if (!current_book) return;
    current_book->remove_current_page();
}
CONTROL_COMMAND(remove_from_book, 0, "Remove current page from current book")

static void move_to_folder (const AnyString& folder) {
    if (!current_book) return;
    auto visible = current_book->visible_range();
    if (!size(visible)) return;
    auto& loc = current_book->block.pages[visible.l]->location;
    auto new_path = cat(folder, '/', iri::path_filename(loc.path()));
    fs::rename(iri::to_fs_path(loc), new_path);
}
CONTROL_COMMAND(move_to_folder, 1, "Move current page to a folder")

static void delete_mark () {
    if (!current_book) return;
    liv::delete_mark(*current_book);
}
CONTROL_COMMAND(delete_mark, 0, "Delete mark file that saves book state.")

} // namespace liv::commands
