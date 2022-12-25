#include "gl.h"

#include <iomanip>
#include <sstream>
#include <SDL2/SDL_video.h>
#include "../ayu/describe.h"

using namespace std::literals;

namespace glow {

struct GLFunctionRegistry {
    std::vector<std::pair<void*, const char*>> to_init;
    bool initted = false;
};
static GLFunctionRegistry& registry () {
    static GLFunctionRegistry r;
    return r;
}

void register_gl_function (void* p, const char* name) {
    static GLFunctionRegistry& reg = registry();
    require(!reg.initted);
    reg.to_init.emplace_back(p, name);
}

void init_gl_functions () {
    static GLFunctionRegistry& reg = registry();
    if (reg.initted) return;
    reg.initted = true;

    require_sdl(!SDL_GL_LoadLibrary(NULL));

    for (auto& p : reg.to_init) {
        *reinterpret_cast<void**>(p.first) = require_sdl(SDL_GL_GetProcAddress(p.second));
    }
    reg.to_init.clear();
}

void throw_on_glGetError (
    const char* gl_function,
    std::source_location loc
) {
    GLenum err = p_glGetError<>();
    if (err) throw ayu::X<GLError>(err, gl_function, loc);
}

} using namespace glow;

AYU_DESCRIBE(glow::GLError,
    delegate(base<glow::GlowError>()),
    elems(
        elem(&glow::GLError::error_code),
        elem(&glow::GLError::gl_function),
        elem(&glow::GLError::loc)
    )
)
