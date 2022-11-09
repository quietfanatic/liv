#pragma once

#include "../geo/vec.h"
#include "../uni/common.h"

struct SDL_Window;

namespace wind {

 // A thin wrapper around an SDL_Window.  Calls SDL_CreateWindow on construction
 // and SDL_DestroyWindow on destruction.
struct Window {
    SDL_Window* sdl_window = null;
    void* gl_context = null;
     // By default the window is hidden and OpenGL-enabled.
    Window();
     // Arguments passed to SDL_CreateWindow
    Window (const char* title, int x, int y, int w, int h, uint32 flags);
     // Slightly shorter version
    Window (const char* title, geo::IVec size);
    ~Window();
     // Coerce
    operator SDL_Window* () const { return sdl_window; }
};

} // namespace wind
