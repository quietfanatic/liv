#include "test-environment.h"

#include <SDL2/SDL.h>
#include "common.h"
#include "gl.h"
#include "image.h"

namespace glow {

TestEnvironment::TestEnvironment (geo::IVec size) :
    size(size),
    test_scheme(
        "test",
        []{
            char* base = AS(SDL_GetBasePath());
            String folder = String(base) + "res/base/glow/test";
            SDL_free(base);
            return folder;
        }()
    ),
    window("Test window", size, wind::GLAttributes{.alpha = 8})
{
    glow::init();
}

TestEnvironment::~TestEnvironment () { }

Image TestEnvironment::read_pixels () {
    Image r (size);
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, r.pixels);
    return r;
}

} // namespace glow
