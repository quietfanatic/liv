#include <cstdio>
#include <SDL2/SDL.h>
#include "../base/ayu/common.h"
#include "../base/ayu/resource.h"
#include "../base/ayu/resource-scheme.h"
#include "../base/glow/common.h"
#include "../base/tap/tap.h"
#include "../base/uni/common.h"
#include "app.h"
#include "common.h"

using namespace app;

int main (int argc, char** argv) {
    glow::require_sdl(SDL_SetHint("SDL_HINT_VIDEO_ALLOW_SCREENSAVER", "1"));
    char* base = glow::require_sdl(SDL_GetBasePath());
     // TODO: allow resource schemes to be readonly
    ayu::FileResourceScheme res_scheme ("res", uni::cat(base, + "res"));
    ayu::FileResourceScheme data_scheme ("data", const_cast<const char*>(base));
    SDL_free(base);

    tap::allow_testing(argc, argv);

    UniqueArray<AnyString> args;
    bool list = false;
    bool done_flags = false;
    for (int i = 1; i < argc; i++) {
        auto arg = StaticString::Static(argv[i]);
        if (!done_flags && arg && arg[0] == '-' && arg != "-") {
            if (arg == "--") {
                done_flags = true;
            }
            else if (arg == "--list") {
                list = true;
            }
            else throw ayu::X<ayu::GenericError>(cat("Unrecognized option ", arg));
        }
        else args.emplace_back(arg);
    }

    App app;
    if (list) {
        if (args.size() != 1) {
            throw ayu::X<ayu::GenericError>("Wrong number of arguments given with --list (must be 1)");
        }
        app.open_list(args[0]);
    }
    else {
        app.open_files(move(args));
    }
    app.run();
    return 0;
}
