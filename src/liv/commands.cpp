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

static void quit_ () {
    if (current_app) current_app->stop();
}
Command<quit_> quit (0, "quit", "Quit application");

static void fullscreen_ () {
    if (current_book) {
        current_book->view.window.set_fullscreen(
            !current_book->view.window.is_fullscreen()
        );
    }
}
Command<fullscreen_> fullscreen (0, "fullscreen", "Toggle fullscreen mode");

static void leave_fullscreen_ () {
     // Check if we're already fullscreen to avoid generating a size changed
     // event.
    if (current_book && current_book->view.window.is_fullscreen()) {
        current_book->view.window.set_fullscreen(false);
    }
}
Command<leave_fullscreen_> leave_fullscreen (0, "leave_fullscreen", "Leave fullscreen mode");

static void leave_fullscreen_or_quit_ () {
    if (current_book && current_book->view.window.is_fullscreen()) {
        current_book->view.window.set_fullscreen(false);
    }
    else if (current_app) {
        current_app->stop();
    }
}
Command<leave_fullscreen_or_quit_> leave_fullscreen_or_quit (
    0, "leave_fullscreen_or_quit", "Leave fullscreen mode, or quit app if not in fullscreen mode"
);

static void prompt_command_ () {
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
Command<prompt_command_> prompt_command (0, "prompt_command", "Prompt for a command with a dialog box");

static void say_ (const FormatList& fmt) {
    if (current_book) {
        UniqueString s;
        fmt.write(s, current_book);
        print_utf8(cat(move(s), "\n"));
    }
}
Command<say_> say (1, "say", "Print a formatted string to stdout with a newline.");

 // TODO: allow single parameter
static void message_box_ (const FormatList& title, const FormatList& message) {
    if (current_book) {
        UniqueString t;
        title.write(t, current_book);
        UniqueString m;
        message.write(m, current_book);
        auto res = run({
            "zenity", "--no-markup", cat("--title=", t), "--info", cat("--text=", m)
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
Command<message_box_> message_box (2, "message_box", "Show a message box with formatted title and content");

static void clipboard_text_ (const FormatList& fmt) {
    if (!current_book) return;
    UniqueString text;
    fmt.write(text, current_book);
    SDL_SetClipboardText(text.c_str());
}
Command<clipboard_text_> clipboard_text (1, "clipboard_text", "Set clipboard text with format list");

static void shell_ (const FormatList& fmt) {
    if (!current_book) return;
    UniqueString cmd;
    fmt.write(cmd, current_book);
    shell(cmd.c_str());
}
Command<shell_> shell (1, "shell", "Create a system shell command with a format list and run it.");

 // Not AnyArray because FormatList is not copyable
static void run_ (const UniqueArray<FormatList>& fmts) {
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
Command<run_> run (1, "run", "Run a system command with the command name and each argument from format lists.");

///// ACTION COMMANDS

static void next_ () {
    if (current_book) current_book->next();
}
Command<next_> next (0, "next", "Go to next page or pages");

static void prev_ () {
    if (current_book) current_book->prev();
}
Command<prev_> prev (0, "prev", "Go to previous page or pages");

static void seek_ (int32 count) {
    if (current_book) current_book->seek(count);
}
Command<seek_> seek (1, "seek", "Add given amount to the current page number");

static void go_next_ (Direction dir) {
    if (current_book) current_book->go_next(dir);
}
Command<go_next_> go_next (1, "go_next", "Move one spread count in the given direction");

static void go_ (Direction dir, int32 count) {
    if (current_book) current_book->go(dir, count);
}
Command<go_> go (2, "go", "Move in the given direction by the given number of pages");

static void trap_pointer_ (bool trap) {
    if (current_book) current_book->trap_pointer(trap);
}
Command<trap_pointer_> trap_pointer (1, "trap_pointer", "Set pointer trap mode");

///// LAYOUT COMMANDS

static void spread_count_ (int32 count) {
    if (current_book) current_book->spread_count(count);
}
Command<spread_count_> spread_count (1, "spread_count", "Change number of pages to view at once");

static void spread_direction_ (Direction dir) {
    if (current_book) current_book->spread_direction(dir);
}
Command<spread_direction_> spread_direction (1, "spread_direction", "Change direction to read book in");

static void auto_zoom_mode_ (AutoZoomMode mode) {
    if (current_book) current_book->auto_zoom_mode(mode);
}
Command<auto_zoom_mode_> auto_zoom_mode (1, "auto_zoom_mode", "Set auto zoom mode: fit or original");

static void set_zoom_ (float zoom) {
    if (current_book) current_book->set_zoom(zoom);
}
Command<set_zoom_> set_zoom (1, "set_zoom", "Set zoom to a specific amount");

static void zoom_ (float factor) {
    if (current_book) current_book->zoom(factor);
}
Command<zoom_> zoom (1, "zoom", "Multiply zoom by a factor");

static void align_ (Vec small, Vec large) {
    if (current_book) current_book->align(small, large);
}
Command<align_> align (2, "align", "Set page alignment (small_align and large_align)");

static void orientation_ (Direction o) {
    if (current_book) current_book->orientation(o);
}
Command<orientation_> orientation (1, "orientation", "Set page orientation");

static void reset_layout_ () {
    if (current_book) current_book->reset_layout();
}
Command<reset_layout_> reset_layout (0, "reset_layout", "Reset most layout parameters to default");

static void reset_settings_ () {
    if (current_book) current_book->reset_settings();
}
Command<reset_settings_> reset_settings (0, "reset_settings", "Reset all temporary settings to default");

///// RENDER COMMANDS

static void upscaler_ (Upscaler mode) {
    if (current_book) current_book->upscaler(mode);
}
Command<upscaler_> upscaler (1, "upscaler", "Set the upscaling interpolation mode");

static void downscaler_ (Downscaler mode) {
    if (current_book) current_book->downscaler(mode);
}
Command<downscaler_> downscaler (1, "downscaler", "Set the downscaling interpolation mode");

static void window_background_ (Fill bg) {
    if (current_book) current_book->window_background(bg);
}
Command<window_background_> window_background (1, "window_background", "Change window background fill");

static void transparency_background_ (Fill bg) {
    if (current_book) current_book->transparency_background(bg);
}
Command<transparency_background_> transparency_background (1, "transparency_background", "Change fill behind transparent images");

static void color_range_ (const ColorRange& range) {
    if (current_book) current_book->color_range(range);
}
Command<color_range_> color_range (1, "color_range", "Adjust the color output range with [[rl rh] [gl gh] [bl bh]]");

///// BOOK COMMANDS

static void sort_ (SortMethod method) {
    if (!current_book) return;
    current_book->sort(method);
}
Command<sort_> sort (1, "sort", "Change sort method of current book");

 // TODO: optional argument?
static void add_to_list_ (const AnyString& list, SortMethod sort) {
    if (!current_book) return;
    auto visible = current_book->visible_range();
    if (!size(visible)) return;

    auto loc = iri::from_fs_path(list);
    const IRI& entry = current_book->block.pages[visible.l]->location;
    add_to_list(loc, entry, sort);
}
Command<add_to_list_> add_to_list (2, "add_to_list", "Add current page filename to a list file and sort it");

static void remove_from_list_ (const AnyString& list) {
    if (!current_book) return;
    auto visible = current_book->visible_range();
    if (!size(visible)) return;
    auto loc = iri::from_fs_path(list);
    const IRI& entry = current_book->block.pages[visible.l]->location;
    remove_from_list(loc, entry);
}
Command<remove_from_list_> remove_from_list (1, "remove_from_list", "Remove current page from list file");

static void remove_from_book_ () {
    if (!current_book) return;
    current_book->remove_current_page();
}
Command<remove_from_book_> remove_from_book (0, "remove_from_book", "Remove current page from current book");

static void move_to_folder_ (const AnyString& folder) {
    if (!current_book) return;
    auto visible = current_book->visible_range();
    if (!size(visible)) return;
    auto& loc = current_book->block.pages[visible.l]->location;
    auto new_path = cat(folder, '/', iri::path_filename(loc.path()));
    fs::rename(iri::to_fs_path(loc), new_path);
}
Command<move_to_folder_> move_to_folder (1, "move_to_folder", "Move current page to a folder");

static void delete_mark_ () {
    if (!current_book) return;
    liv::delete_mark(*current_book);
}
Command<delete_mark_> delete_mark (0, "delete_mark", "Delete mark file that saves book state.");

} // namespace liv::commands
