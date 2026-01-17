#include "commands.h"

#include <algorithm>
#include <SDL2/SDL_clipboard.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_messagebox.h>
#include "../dirt/uni/io.h"
#include "../dirt/uni/shell.h"
#include "../dirt/uni/text.h"
#include "app.h"
#include "book.h"
#include "list.h"
#include "mark.h"
#include "settings.h"

AYU_DESCRIBE(liv::Command)
AYU_DESCRIBE(liv::Statement,
    delegate(base<control::Statement<liv::Command>>())
)

namespace liv::commands {

///// GENERIC COMMANDS
static void echo (Book&, const AnyString& text) {
    uni::print_utf8(text);
}
CONTROL_COMMAND_FUNCTION(Command, echo, 1)

static void seq (Book& book, UniqueArray<Statement>& sts) {
    for (auto& st : sts) st(book);
}
CONTROL_COMMAND_COLLAPSED(Command, seq)

static void toggle (Book& book, Statement& a, Statement& b, bool& flag) {
    ((flag = !flag) ? b : a)(book);
}
CONTROL_COMMAND_FUNCTION(Command, toggle, 2)

///// APP AND WINDOW COMMANDS

static void quit (Book&) {
    if (current_app) current_app->stop();
}
CONTROL_COMMAND_FUNCTION(Command, quit, 0)

static void fullscreen (Book& book) {
    book.view.window.set_fullscreen(
        !book.view.window.is_fullscreen()
    );
}
CONTROL_COMMAND_FUNCTION(Command, fullscreen, 0)

static void leave_fullscreen (Book& book) {
     // Check if we're already fullscreen to avoid generating a size changed
     // event.
    if (book.view.window.is_fullscreen()) {
        book.view.window.set_fullscreen(false);
    }
}
CONTROL_COMMAND_FUNCTION(Command, leave_fullscreen, 0)

static void leave_fullscreen_or_quit (Book& book) {
    if (book.view.window.is_fullscreen()) {
        book.view.window.set_fullscreen(false);
    }
    else if (current_app) {
        current_app->stop();
    }
}
CONTROL_COMMAND_FUNCTION(Command, leave_fullscreen_or_quit, 0)

static void prompt_command (Book& book) {
    auto last = book.state.settings->get(
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
                book.view.window
            );
        }
        return;
    }
    AnyString text = move(res.out);
    if (text && text.back() == '\n') text.pop_back();
    book.state.settings->window.last_prompt_command = text;
    book.need_mark = true;
    try {
        Statement cmd;
        ayu::item_from_list_string(&cmd, text);
        cmd(book);
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
CONTROL_COMMAND_FUNCTION(Command, prompt_command, 0)

static void say (Book& book, const FormatList& fmt) {
    UniqueString s;
    fmt.write(s, &book);
    print_utf8(cat(move(s), "\n"));
}
CONTROL_COMMAND_FUNCTION(Command, say, 1)

 // TODO: allow single parameter
static void message_box (
    Book& book, const FormatList& title, const FormatList& message
) {
    UniqueString t;
    title.write(t, &book);
    UniqueString m;
    message.write(m, &book);
    auto res = run({
        "zenity", "--no-markup", cat("--title=", t), "--info", cat("--text=", m)
    });
    if (res.command_wasnt_found()) {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_INFORMATION,
            t.c_str(), m.c_str(),
            book.view.window
        );
    }
}
CONTROL_COMMAND_FUNCTION(Command, message_box, 2)

static void clipboard_text (Book& book, const FormatList& fmt) {
    UniqueString text;
    fmt.write(text, &book);
    SDL_SetClipboardText(text.c_str());
}
CONTROL_COMMAND_FUNCTION(Command, clipboard_text, 1)

static void shell (Book& book, const FormatList& fmt) {
    UniqueString cmd;
    fmt.write(cmd, &book);
    uni::shell(cmd.c_str());
}
CONTROL_COMMAND_FUNCTION(Command, shell, 1)

 // Not AnyArray because FormatList is not copyable
static void run (Book& book, const UniqueArray<FormatList>& fmts) {
    auto args = UniqueArray<UniqueString>(
        fmts.size(), [&](u32 i)
    {
        UniqueString s;
        fmts[i].write(s, &book);
        return s;
    });
     // TODO: we can probably .reinterpret<>()
    auto strs = UniqueArray<Str>(args.size(), [&args](u32 i){
        return Str(args[i]);
    });
    run(strs);
}
CONTROL_COMMAND_COLLAPSED(Command, run)

///// ACTION COMMANDS

static void next (Book& book) { book.next(); }
CONTROL_COMMAND_FUNCTION(Command, next, 0)

static void prev (Book& book) { book.prev(); }
CONTROL_COMMAND_FUNCTION(Command, prev, 0)

static void seek (Book& book, int32 count) { book.seek(count); }
CONTROL_COMMAND_FUNCTION(Command, seek, 1)

static void go_next (Book& book, Direction dir) { book.go_next(dir); }
CONTROL_COMMAND_FUNCTION(Command, go_next, 1)

static void go (Book& book, Direction dir, int32 count) {
    book.go(dir, count);
}
CONTROL_COMMAND_FUNCTION(Command, go, 2)

static void trap_pointer (Book& book, bool trap) {
    book.trap_pointer(trap);
}
CONTROL_COMMAND_FUNCTION(Command, trap_pointer, 1)

///// LAYOUT COMMANDS

static void spread_count (Book& book, int32 count) {
    book.spread_count(count);
}
CONTROL_COMMAND_FUNCTION(Command, spread_count, 1)

static void spread_direction (Book& book, Direction dir) {
    book.spread_direction(dir);
}
CONTROL_COMMAND_FUNCTION(Command, spread_direction, 1)

static void auto_zoom_mode (Book& book, AutoZoomMode mode) {
    book.auto_zoom_mode(mode);
}
CONTROL_COMMAND_FUNCTION(Command, auto_zoom_mode, 1)

static void set_zoom (Book& book, float zoom) {
    book.set_zoom(zoom);
}
CONTROL_COMMAND_FUNCTION(Command, set_zoom, 1)

static void zoom (Book& book, float factor) {
    book.zoom(factor);
}
CONTROL_COMMAND_FUNCTION(Command, zoom, 1)

static void align (Book& book, Vec small, Vec large) {
    book.align(small, large);
}
CONTROL_COMMAND_FUNCTION(Command, align, 2)

static void orientation (Book& book, Direction o) {
    book.orientation(o);
}
CONTROL_COMMAND_FUNCTION(Command, orientation, 1)

static void reset_layout (Book& book) {
    book.reset_layout();
}
CONTROL_COMMAND_FUNCTION(Command, reset_layout, 0)

static void reset_settings (Book& book) {
    book.reset_settings();
}
CONTROL_COMMAND_FUNCTION(Command, reset_settings, 0)

///// RENDER COMMANDS

static void upscaler (Book& book, Upscaler mode) {
    book.upscaler(mode);
}
CONTROL_COMMAND_FUNCTION(Command, upscaler, 1)

static void deringer (Book& book, Deringer mode) {
    book.deringer(mode);
}
CONTROL_COMMAND_FUNCTION(Command, deringer, 1)

static void downscaler (Book& book, Downscaler mode) {
    book.downscaler(mode);
}
CONTROL_COMMAND_FUNCTION(Command, downscaler, 1)

static void window_background (Book& book, Fill bg) {
    book.window_background(bg);
}
CONTROL_COMMAND_FUNCTION(Command, window_background, 1)

static void transparency_background (Book& book, Fill bg) {
    book.transparency_background(bg);
}
CONTROL_COMMAND_FUNCTION(Command, transparency_background, 1)

static void color_range (Book& book, const ColorRange& range) {
    book.color_range(range);
}
CONTROL_COMMAND_FUNCTION(Command, color_range, 1)

///// BOOK COMMANDS

static void sort (Book& book, SortMethod method) {
    book.sort(method);
}
CONTROL_COMMAND_FUNCTION(Command, sort, 1)

 // TODO: optional argument?
static void add_to_list (Book& book, const AnyString& list, SortMethod sort) {
    auto visible = book.visible_range();
    if (!size(visible)) return;

    auto loc = iri::from_fs_path(list);
    const IRI& entry = book.block.pages[visible.l]->location;
    add_to_list(loc, entry, sort);
}
CONTROL_COMMAND_FUNCTION(Command, add_to_list, 2)

static void remove_from_list (Book& book, const AnyString& list) {
    auto visible = book.visible_range();
    if (!size(visible)) return;
    auto loc = iri::from_fs_path(list);
    const IRI& entry = book.block.pages[visible.l]->location;
    liv::remove_from_list(loc, entry);
}
CONTROL_COMMAND_FUNCTION(Command, remove_from_list, 1)

static void remove_from_book (Book& book) {
    book.remove_current_page();
}
CONTROL_COMMAND_FUNCTION(Command, remove_from_book, 0)

static void move_to_folder (Book& book, const AnyString& folder) {
    auto visible = book.visible_range();
    if (!size(visible)) return;
    auto& loc = book.block.pages[visible.l]->location;
    auto new_path = cat(folder, '/', iri::path_filename(loc.path()));
    fs::rename(iri::to_fs_path(loc), new_path);
}
CONTROL_COMMAND_FUNCTION(Command, move_to_folder, 1)

static void delete_mark (Book& book) {
    liv::delete_mark(book);
}
CONTROL_COMMAND_FUNCTION(Command, delete_mark, 0)

} // liv::commands

