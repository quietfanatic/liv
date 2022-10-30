#pragma once

#include "../base/geo/rect.h"
#include "../base/geo/vec.h"

namespace app {

enum FitMode {
    FIT,
    STRETCH,
    MANUAL
};

struct View {
    FitMode fit_mode = FIT;
     // These are only meaningful if fit_mode is MANUAL
    float zoom = 1.0;
     // Relative to window, bottom-left origin
    geo::Vec offset;

    bool fullscreen = false;

    geo::Rect page_position (geo::Vec page_size, geo::Vec window_size) const;
};

} // namespace app
