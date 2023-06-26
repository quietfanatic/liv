#include "common.h"

#include <cstdlib>
#include <iostream>
#include <SDL2/SDL_error.h>
#include "../ayu/describe.h"
#include "gl.h"

namespace glow {

void init () {
    init_gl_functions();
}

[[gnu::cold]]
void requirement_failed_sdl (std::source_location loc) {
     // TODO: use cat
    std::cerr << "ERROR: require_sdl() failed at "s << loc.file_name()
              << ':' << loc.line() << "\n       in " << loc.function_name()
              << "\n       SDL_GetError() == " << SDL_GetError() << std::endl;
    std::abort();
}

} using namespace glow;

AYU_DESCRIBE(glow::GlowError,
    delegate(base<ayu::Error>())
)

