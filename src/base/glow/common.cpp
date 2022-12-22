#include "common.h"

#include <SDL2/SDL_error.h>
#include "../ayu/describe.h"
#include "gl.h"

namespace glow {

void init () {
    init_gl_functions();
}

void assert_failed_sdl (const char* function, const char* filename, uint line) {
    throw ayu::X<AssertionFailedSDL>(function, filename, line, SDL_GetError());
}

} using namespace glow;

AYU_DESCRIBE(glow::GlowError,
    delegate(base<ayu::Error>())
)

AYU_DESCRIBE(glow::AssertionFailedSDL,
    delegate(base<ayu::Error>()),
    elems(
        elem(&AssertionFailedSDL::function),
        elem(&AssertionFailedSDL::filename),
        elem(&AssertionFailedSDL::line),
        elem(&AssertionFailedSDL::sdl_error)
    )
)
