#include <string>
#include <string_view>
#include <SDL2/SDL.h>
#include "base/glow/image.h"
#include "base/hacc/resource.h"
#include "base/tap/tap.h"
#include "editor.h"

using namespace std::literals;
using namespace pokey;

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

    wind::Window window;
    window.open();
    glow::init();
    for (;;) {
        SDL_Event event;
        AS(SDL_WaitEvent(&event));
        switch (event.type) {
            case SDL_QUIT: {
                window.close();
                return;
            }
            case SDL_KEYDOWN: {
                switch (event.key.keysym.scancode) {
                    case 0x29: { // Escape
                        window.close();
                        return;
                    }
                    default: break;
                }
            }
            default: break;
        }
    }
}
