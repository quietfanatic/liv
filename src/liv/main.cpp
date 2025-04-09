#include <cstdio>
#include <SDL2/SDL.h>
#include "../dirt/ayu/common.h"
#include "../dirt/ayu/resources/resource.h"
#include "../dirt/ayu/resources/scheme.h"
#include "../dirt/glow/common.h"
#include "../dirt/iri/path.h"
#include "../dirt/tap/tap.h"
#include "../dirt/uni/common.h"
#include "../dirt/uni/io.h"
#include "../dirt/uni/shell.h"
#include "../dirt/uni/strings.h"
#include "app.h"
#include "common.h"
#include "settings.h"

using namespace liv;

int main (int argc, char** argv) {
    plog("main");
    glow::require_sdl(SDL_SetHint("SDL_HINT_VIDEO_ALLOW_SCREENSAVER", "1"));
    char* base = glow::require_sdl(SDL_GetBasePath());
     // TODO: allow resource schemes to be readonly
    ayu::FolderResourceScheme res_scheme ("res", cat(base, + "res"));
    ayu::FolderResourceScheme data_scheme ("data", UniqueString(base));
    free(base);
    plog("set up");
#ifndef TAP_DISABLE_TESTS
    tap::allow_testing(argc, argv);
#endif
    UniqueArray<AnyString> args;
    bool help = false;
    bool list = false;
    bool done_flags = false;
    std::optional<SortMethod> sort;
     // TODO: --no-mark, --data-folder, --settings
    for (int i = 1; i < argc; i++) {
        auto arg = StaticString(argv[i]);
        if (!done_flags && arg && arg[0] == '-' && arg != "-") {
            if (arg == "--") {
                done_flags = true;
            }
            if (arg == "--help" || arg == "-help" || arg == "-h") {
                help = true;
            }
            else if (arg == "--list") {
                list = true;
            }
            else if (arg.slice(0, 7) == "--sort=") {
                sort.emplace();
                ayu::item_from_list_string(&*sort, arg.slice(7));
            }
            else raise(e_General, cat("Unrecognized option ", arg));
        }
        else args.emplace_back(arg);
    }
    plog("parsed args");

    if (help) {
        print_utf8(
R"(liv <options> [--] <filenames>
    --help: Print this help message
    --list: Read a list of filenames, one per line.  Use - for stdin.
    --sort=<criterion>,<flags...>: Sort files.  <criterion> is one of:
            natural unicode last_modified file_size shuffle unsorted
        and <flags...> is zero or more of:
            reverse not_args not_lists
        See res/liv/settings-default.ayu for more documentation.
)"
        );
        return 255;
    }

    try {
        auto settings = std::make_unique<Settings>();
        settings->files.sort = sort;
        App app;
        if (list) {
            if (args.size() != 1) {
                raise(e_General,
                    "Wrong number of arguments given with --list (must be 1)"
                );
            }
            app.open_list(args[0], move(settings));
        }
        else {
            app.open_args(move(args), move(settings));
        }
        plog("opened args");
        app.run();
    }
    catch (std::exception& e) {
        auto res = run({
            "zenity", "--error", "--title=LIV Error", "--no-markup",
                cat("--text=Uncaught exception: ", e.what())
        });
        if (res.command_wasnt_found()) {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_ERROR,
                "LIV Error",
                cat("Uncaught exceptin: ", e.what(), '\0').data(),
                null
            );
        }
        throw;
    }
    return 0;
}
