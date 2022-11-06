#pragma once

#include "../base/control/command.h"
#include "../base/control/input.h"
#include "../base/geo/vec.h"

namespace app {

 // TODO: Add more fit modes
enum AutoZoomMode : uint8 {
    FIT,
    FIT_WIDTH,
    FIT_HEIGHT,
    FILL,
    ORIGINAL
};

enum InterpolationMode : uint8 {
    NEAREST,
    LINEAR,
    CUBIC,
    CUBIC_NOHALO
};

struct Mapping {
    control::Input input;
    control::Statement action;
};

struct Settings {
    struct Page {
        AutoZoomMode auto_zoom_mode = FIT;
        float max_zoom = 32;
        float min_page_size = 16;
        bool reset_zoom_on_page_turn = true;
        geo::Vec small_align = {0.5, 0.5};
        geo::Vec large_align = {0.5, 0.5};
        InterpolationMode interpolation_mode = CUBIC_NOHALO;
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
