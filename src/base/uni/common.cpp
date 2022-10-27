#include "common.h"

#include <SDL2/SDL_error.h>
#include "../hacc/haccable.h"

using namespace std::literals;
using namespace uni;

void assert_failed_general (const char* function, const char* filename, uint line) {
    throw X::AssertionFailed(function, filename, line);
}
void assert_failed_sdl (const char* function, const char* filename, uint line) {
    throw X::AssertionFailedSDL(function, filename, line, SDL_GetError());
}

HACCABLE(uni::X::AssertionFailed,
    elems(
        elem(&X::AssertionFailed::function),
        elem(&X::AssertionFailed::filename),
        elem(&X::AssertionFailed::line)
    )
)

HACCABLE(uni::X::AssertionFailedSDL,
    elems(
        elem(&X::AssertionFailedSDL::function),
        elem(&X::AssertionFailedSDL::filename),
        elem(&X::AssertionFailedSDL::line),
        elem(&X::AssertionFailedSDL::mess)
    )
)