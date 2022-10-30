#pragma once

namespace app {

enum FitMode {
    FIT,
    STRETCH,
    MANUAL
};

struct View {
    FitMode fit_mode = FIT;
    float zoom = 1.0;
};

} // namespace app
