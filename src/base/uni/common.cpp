#include "common.h"

#if __has_include(<SDL2/SDL_error.h>)
#include <SDL2/SDL_error.h>
#endif

#include "../ayu/describe.h"

namespace uni {

void assert_failed_general (const char* function, const char* filename, uint line) {
    throw X::AssertionFailed(function, filename, line);
}
void assert_failed_sdl (const char* function, const char* filename, uint line) {
#if __has_include(<SDL2/SDL_error.h>)
    throw X::AssertionFailedSDL(function, filename, line, SDL_GetError());
#else
    throw X::AssertionFailedSDL(function, filename, line, "(SDL headers not available for error info)");
#endif
}

} using namespace uni;

AYU_DESCRIBE(uni::X::AssertionFailed,
    delegate(base<ayu::X::Error>()),
    elems(
        elem(&X::AssertionFailed::function),
        elem(&X::AssertionFailed::filename),
        elem(&X::AssertionFailed::line)
    )
)

AYU_DESCRIBE(uni::X::AssertionFailedSDL,
    delegate(base<ayu::X::Error>()),
    elems(
        elem(&X::AssertionFailedSDL::function),
        elem(&X::AssertionFailedSDL::filename),
        elem(&X::AssertionFailedSDL::line),
        elem(&X::AssertionFailedSDL::sdl_error)
    )
)
