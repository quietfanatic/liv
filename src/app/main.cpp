#include <SDL2/SDL.h>
#include "../base/ayu/common.h"
#include "../base/ayu/resource.h"
#include "../base/ayu/resource-scheme.h"
#include "../base/tap/tap.h"
#include "../base/uni/common.h"
#include "app.h"

int main (int argc, char** argv) {
    AS(SDL_SetHint("SDL_HINT_VIDEO_ALLOW_SCREENSAVER", "1"));
    char* base = AS(SDL_GetBasePath());
    ayu::FileResourceScheme res_scheme ("res", String(base) + "res");
    SDL_free(base);

    tap::allow_testing(argc, argv);

    std::vector<std::string> args;
    bool done_flags = false;
    for (int i = 1; i < argc; i++) {
        if (!done_flags && argv[i][0] == '-') {
            if (argv[i] == "--"sv) {
                done_flags = true;
            }
            else throw ayu::X::GenericError("This program doesn't support options");
        }
        else args.emplace_back(argv[i]);
    }

    app::App app;
    app.open_files(args);
    app.run();
    return 0;
}
