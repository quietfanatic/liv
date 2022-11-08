#pragma once

#include <functional>
#include "../geo/vec.h"
#include "../uni/common.h"

namespace wind {

 // An active loop using SDL.  The default on_step calls poll_events.
struct ActiveLoop {
     // Desired framerate.
    double fps = 60;
     // If lag is less than this amount in frames, slow down instead of dropping
     //  frames.  This will allow playing on monitors vsynced to 59.9hz or
     //  something like that without dropping any frames.
    double min_lag_tolerance = 0.005;  // 60 -> 59.7
     // If lag is more than this amount in frames, slow down instead of dropping
     //  frames.  The game will be barely playable, but it's better than being
     //  frozen.
    double max_lag_tolerance = 3.0;

     // Will be called at the desired fps, unless slowdown happens.
     // If on_step is null, then on step the loop will quit on SDL_QUIT and
     // send other events through process_window_event (see window.h).
    std::function<void()> on_step = null;
     // Will be called at the desired fps, unless frameskip or slowdown happens.
     // If on_draw is null, then on draw the loop will do nothing.
    std::function<void()> on_draw = null;

     // stop() has been called.
    bool stop_requested = false;

     // Loops over update and draw until stop is called
    void start ();
     // Makes start() return.
    void stop ();
};

} // namespace wind
