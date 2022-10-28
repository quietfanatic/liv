#include <string>
#include <string_view>
#include <SDL2/SDL.h>
#include "../base/glow/common.h"
#include "../base/glow/file-texture.h"
#include "../base/glow/texture-program.h"
#include "../base/hacc/resource.h"
#include "../base/tap/tap.h"
#include "../base/uni/common.h"
#include "../base/wind/window.h"
#include "book.h"

using namespace app;

int main (int argc, char** argv) {
    char* base = AS(SDL_GetBasePath());
    hacc::set_file_resource_root(String(base) + "res");
    SDL_free(base);

    tap::allow_testing(argc, argv);

    std::vector<std::string> args;
    bool done_flags = false;
    for (int i = 1; i < argc; i++) {
        if (!done_flags && argv[i][0] == '-') {
            if (argv[i] == "--"sv) {
                done_flags = true;
            }
            else throw hacc::X::GenericError("This program doesn't support options");
        }
        else args.emplace_back(argv[i]);
    }

    Book book (args);
    book.draw();

    for (;;) {
        SDL_Event event;
        AS(SDL_WaitEvent(&event));
        if (event.type == SDL_QUIT) {
            return 0;
        }
        wind::process_window_event(&event);
    }
}
