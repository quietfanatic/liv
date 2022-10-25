#pragma once

#include <functional>
#include "../geo/vec.h"
#include "../uni/common.h"

union SDL_Event;
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

    std::function<bool(SDL_Event*)> on_event = null;

     // Updates window parameters
    void update ();

     // Creates window and makes it visible
    void open ();
     // Closes window
    void close ();

    ~Window();
};

 // Finds the window this event belongs to and calls its on_event.
 // This includes window events, keyboard and mouse events, and user events if
 // they define a WindowID.  Does not include joystick/controller events, and
 // only includes basic touch events.  Returns true if the event was mapped to
 // a window and its on_event returned true.
bool process_window_event (SDL_Event*);

} // namespace wind
