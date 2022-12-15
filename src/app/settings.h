#pragma once

#include <vector>
#include "../base/control/command.h"
#include "../base/control/input.h"
#include "../base/geo/vec.h"
#include "../base/glow/colors.h"
#include "../base/uni/common.h"
#include "common.h"

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
    SMOOTHED,
    CUBIC,
    SMART_CUBIC
};

struct Fill : glow::RGBA8 { using glow::RGBA8::RGBA8; };

constexpr Fill BLACK = {0, 0, 0, 255};
constexpr Fill WHITE = {255, 255, 255, 255};
constexpr Fill TRANSPARENT = {0, 0, 0, 0};

struct Mapping {
    control::Input input;
    control::Statement action;
};

struct WindowSettings {
    std::optional<geo::IVec> size;
    std::optional<bool> fullscreen;
    std::optional<Fill> window_background;
};
struct LayoutSettings {
    std::optional<isize> spread_pages;
    std::optional<isize> max_spread_pages;
    std::optional<AutoZoomMode> auto_zoom_mode;
    std::optional<float> max_zoom;
    std::optional<float> min_page_size;
    std::optional<bool> reset_zoom_on_page_turn;
    std::optional<geo::Vec> small_align;
    std::optional<geo::Vec> large_align;
};
struct PageSettings {
    std::optional<InterpolationMode> interpolation_mode;
};
struct ControlSettings {
    std::optional<float> drag_speed;
};
struct FilesSettings {
    std::optional<std::set<String>> supported_extensions;
};
struct MemorySettings {
    std::optional<uint32> preload_ahead;
    std::optional<uint32> preload_behind;
    std::optional<double> page_cache_mb;
};

 // Using inheritance instead of containment because it makes using member
 // pointers much simpler.
struct Settings :
    WindowSettings, LayoutSettings, PageSettings,
    ControlSettings, FilesSettings, MemorySettings
{
    std::vector<Mapping> mappings;
};

extern const Settings builtin_default_settings;
extern const Settings* res_default_settings;

void init_settings ();

template <class T, class Category>
const T& get_setting (
    const Settings* app_settings,
    std::optional<T> Category::* setting
) {
    init_settings();
    auto setting_generic = static_cast<std::optional<T> Settings::*>(setting);
    if (app_settings && app_settings->*setting_generic) {
        return *(app_settings->*setting_generic);
    }
    else if (res_default_settings->*setting_generic) {
        return *(res_default_settings->*setting_generic);
    }
    else return *(builtin_default_settings.*setting_generic);
}

} // namespace app
