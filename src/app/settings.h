#pragma once

#include <optional>
#include "../dirt/control/command.h"
#include "../dirt/control/input.h"
#include "../dirt/geo/vec.h"
#include "../dirt/glow/colors.h"
#include "../dirt/uni/common.h"
#include "../dirt/uni/strings.h"
#include "common.h"

namespace app {

struct Fill : glow::RGBA8 { using glow::RGBA8::RGBA8; };

constexpr Fill BLACK = {0, 0, 0, 255};
constexpr Fill WHITE = {255, 255, 255, 255};
constexpr Fill TRANSPARENT = {0, 0, 0, 0};

enum SpreadDirection : uint8 {
    RIGHT,
    LEFT,
    DOWN,
    UP
};

enum AutoZoomMode : uint8 {
    FIT,
    FIT_WIDTH,
    FIT_HEIGHT,
    ORIGINAL
};

enum InterpolationMode : uint8 {
    NEAREST,
    LINEAR,
    SMOOTHED,
    CUBIC,
    SMART_CUBIC
};

enum TrimMode : uint8 {
    TRIM_NONE,
    TRIM_PAGE_CACHE
};

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
    std::optional<int32> spread_count;
    std::optional<SpreadDirection> spread_direction;
    std::optional<AutoZoomMode> auto_zoom_mode;
    std::optional<float> max_zoom;
    std::optional<float> min_zoomed_size;
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
    std::optional<std::set<AnyString>> supported_extensions;
};
struct MemorySettings {
    std::optional<uint32> preload_ahead;
    std::optional<uint32> preload_behind;
    std::optional<double> page_cache_mb;
    std::optional<TrimMode> trim_when_minimized;
};

 // Using inheritance instead of containment because it makes using member
 // pointers much simpler.
struct Settings :
    WindowSettings, LayoutSettings, PageSettings,
    ControlSettings, FilesSettings, MemorySettings
{
    UniqueArray<Mapping> mappings;
    template <class T, class Category>
    const T& get (std::optional<T> Category::*) const;
};

extern const Settings builtin_default_settings;
extern const Settings* res_default_settings;

void init_settings ();

template <class T, class Category>
const T& Settings::get (
    std::optional<T> Category::* setting
) const {
    init_settings();
    auto setting_generic = static_cast<std::optional<T> Settings::*>(setting);
    if (this->*setting_generic) {
        return *(this->*setting_generic);
    }
    else if (res_default_settings->*setting_generic) {
        return *(res_default_settings->*setting_generic);
    }
    else return *(builtin_default_settings.*setting_generic);
}

} // namespace app
