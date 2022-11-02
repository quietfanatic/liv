#pragma once

#include "../base/control/command.h"
#include "../base/control/input.h"
#include "../base/geo/vec.h"

namespace app {

 // TODO: Add more fit modes
enum FitMode : uint8 {
    FIT,
    STRETCH,
    MANUAL
};

struct Mapping {
    control::Input input;
    control::Statement action;
};

struct Settings {
    struct Page {
        FitMode fit_mode = FIT;
    } page;
    struct Window {
        geo::IVec size = {720, 720};
        bool fullscreen = false;
    } window;
    struct Memory {
        uint32 preload_ahead = 1;
        uint32 preload_behind = 1;
        double page_cache_mb = 200;
    } memory;
    std::vector<Mapping> mappings;
};

} // namespace app
