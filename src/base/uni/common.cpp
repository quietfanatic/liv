#include "common.h"

#if __has_include(<SDL2/SDL_error.h>)
#include <SDL2/SDL_error.h>
#endif

#include "../ayu/describe.h"

namespace uni {

void assert_failed_general (const char* function, const char* filename, uint line) {
    throw ayu::X<AssertionFailed>(function, filename, line);
}
void assert_failed_sdl (const char* function, const char* filename, uint line) {
#if __has_include(<SDL2/SDL_error.h>)
    throw ayu::X<AssertionFailedSDL>(function, filename, line, SDL_GetError());
#else
    throw ayu::X<AssertionFailedSDL>(function, filename, line, "(SDL headers not available for error info)");
#endif
}

} using namespace uni;

AYU_DESCRIBE(uni::AssertionFailed,
    delegate(base<ayu::Error>()),
    elems(
        elem(&AssertionFailed::function),
        elem(&AssertionFailed::filename),
        elem(&AssertionFailed::line)
    )
)

AYU_DESCRIBE(uni::AssertionFailedSDL,
    delegate(base<ayu::Error>()),
    elems(
        elem(&AssertionFailedSDL::function),
        elem(&AssertionFailedSDL::filename),
        elem(&AssertionFailedSDL::line),
        elem(&AssertionFailedSDL::sdl_error)
    )
)
