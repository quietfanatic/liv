#include "common.h"

#include "../ayu/describe.h"
#include "gl.h"

namespace glow {

void init () {
    init_gl_functions();
}

} using namespace glow;

AYU_DESCRIBE(glow::X::GlowError,
    delegate(base<ayu::X::Error>())
)
