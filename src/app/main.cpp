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
    ayu::FileResourceScheme res_scheme ("res", std::string(base) + "res");
    ayu::FileResourceScheme data_scheme ("data", std::string(base));
    SDL_free(base);

    tap::allow_testing(argc, argv);

    std::vector<std::string> args;
    bool list = false;
    bool done_flags = false;
    for (int i = 1; i < argc; i++) {
        if (!done_flags && argv[i][0] == '-') {
            if (argv[i] == "--"sv) {
                done_flags = true;
            }
            else if (argv[i] == "-"sv) {
                args.emplace_back(argv[i]);
            }
            else if (argv[i] == "--list"sv) {
                list = true;
            }
            else throw ayu::X<ayu::GenericError>("Unrecognized option " + std::string(argv[i]));
        }
        else args.emplace_back(argv[i]);
    }

    App app;
    if (list) {
        if (args.size() != 1) {
            throw ayu::X<ayu::GenericError>("Wrong number of arguments given with --list (must be 1)");
        }
        app.open_list(args[0]);
    }
    else {
        app.open_files(std::move(args));
    }
    app.run();
    return 0;
}
