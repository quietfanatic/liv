#pragma once

#include "../geo/vec.h"
#include "../uni/common.h"

struct SDL_Window;

namespace wind {
using namespace uni;

 // Attributes to give to SDL_GL_SetAttribute before creating the window
struct GLAttributes {
    int red = 8;
    int green = 8;
    int blue = 8;
    int alpha = 0;
    int depth = 0;
    int stencil = 0;
};

 // A thin wrapper around an SDL_Window.  Calls SDL_CreateWindow on construction
 // and SDL_DestroyWindow on destruction.
struct Window {
    SDL_Window* sdl_window = null;
    void* gl_context = null;
     // By default the window is hidden and OpenGL-enabled.
    Window(const GLAttributes& attrs = GLAttributes());
     // Arguments passed to SDL_CreateWindow
    Window (
        const char* title,
        int x, int y, int w, int h, uint32 flags,
        const GLAttributes& attrs = GLAttributes()
    );
     // Slightly shorter version
    Window (
        const char* title, geo::IVec size,
        const GLAttributes& attrs = GLAttributes()
    );
    ~Window();
     // Coerce
    operator SDL_Window* () const { return sdl_window; }
};

} // namespace wind
