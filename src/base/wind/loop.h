#pragma once

#include <functional>
#include "../geo/vec.h"
#include "../uni/common.h"

namespace wind {

struct Loop {
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

    std::function<void()> step = []{};
    std::function<void()> draw = []{};

    bool stop_requested = false;

     // Loops over update and draw until stop is called
    void start ();
     // Makes start() return.
    void stop ();
};

} // namespace wind
