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
            char* base = require_sdl(SDL_GetBasePath());
            auto folder = cat(base, "res/dirt/glow/test");
            SDL_free(base);
            return folder;
        }()
    ),
    window("Test window", size, wind::GLAttributes{.alpha = 8})
{
     // Some gl drivers won't render to hidden windows, so do our best to hide
     // the window manually
    SDL_MinimizeWindow(window);
    SDL_ShowWindow(window);
    SDL_MinimizeWindow(window);
    glow::init();
     // Make sure we got a window of the correct size
    int w; int h;
    SDL_GetWindowSize(window, &w, &h);
    require(w == size.x && h == size.y);
}

TestEnvironment::~TestEnvironment () { }

Image TestEnvironment::read_pixels () {
    Image r (size);
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, r.pixels);
    return r;
}

} // namespace glow
