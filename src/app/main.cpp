#include <cstdio>
#include <SDL2/SDL.h>
#include "../dirt/ayu/common.h"
#include "../dirt/ayu/resources/resource.h"
#include "../dirt/ayu/resources/scheme.h"
#include "../dirt/glow/common.h"
#include "../dirt/tap/tap.h"
#include "../dirt/uni/common.h"
#include "../dirt/uni/io.h"
#include "app.h"
#include "common.h"

using namespace app;

int main (int argc, char** argv) {
    glow::require_sdl(SDL_SetHint("SDL_HINT_VIDEO_ALLOW_SCREENSAVER", "1"));
    char* base = glow::require_sdl(SDL_GetBasePath());
     // TODO: allow resource schemes to be readonly
    ayu::FileResourceScheme res_scheme ("res", uni::cat(base, + "res"));
    ayu::FileResourceScheme data_scheme ("data", UniqueString(base));
    free(base);

    tap::allow_testing(argc, argv);

    UniqueArray<AnyString> args;
    bool help = false;
    bool list = false;
    bool done_flags = false;
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
            else raise(e_General, cat("Unrecognized option ", arg));
        }
        else args.emplace_back(arg);
    }

    App app;
    if (help) {
        warn_utf8(
R"(liv <options> [--] <filenames>
    --help: Print this help message
    --list: Read a list of filenames, one per line.  Use - for stdin.
)"
        );
        return 1;
    }
    if (list) {
        if (args.size() != 1) {
            raise(e_General,
                "Wrong number of arguments given with --list (must be 1)"
            );
        }
        app.open_list(args[0]);
    }
    else {
        app.open_files(move(args));
    }
    app.run();
    return 0;
}
