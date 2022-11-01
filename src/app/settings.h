#pragma once

#include "../base/control/command.h"
#include "../base/control/input.h"
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
     // In pixels, relative to window, bottom-left origin
    geo::Vec offset;
    bool fullscreen = false;
    geo::IVec window_size = {720, 720};
};

struct Mapping {
    control::Input input;
    control::Statement action;
};

struct Settings {
    View default_view;
    std::vector<Mapping> mappings;
};

} // namespace app
