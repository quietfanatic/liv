#pragma once

#include <functional>
#include "../geo/vec.h"
#include "../uni/common.h"

struct SDL_Window;

namespace wind {
using namespace geo;

struct Window {
     // Window title
    String title;
     // Width and height in...pixels?
     // TODO: investigate what happens in HiDPI
    IVec size = {640, 480};
     // Allow window to be resized by user.  width and height will be updated.
    bool resizable = false;
     // Window will exist but will not be visible.  Useful for testing.
    bool hidden = false;

    SDL_Window* sdl_window = null;
    void* gl_context = null;

     // Updates window parameters
    void update ();

     // Creates window and makes it visible
    void open ();
     // Closes window
    void close ();

    ~Window();
};

} // namespace wind
