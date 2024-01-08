#pragma once

#include <optional>
#include "../dirt/control/command.h"
#include "../dirt/control/input.h"
#include "../dirt/geo/vec.h"
#include "../dirt/glow/colors.h"
#include "../dirt/uni/common.h"
#include "../dirt/uni/strings.h"
#include "common.h"
#include "format.h"
#include "sort.h"

namespace liv {

struct Fill : glow::RGBA8 {
    using glow::RGBA8::RGBA8;

    static const Fill Black;
    static const Fill White;
    static const Fill Transparent;
};
constexpr Fill Fill::Black = {0, 0, 0, 255};
constexpr Fill Fill::White = {255, 255, 255, 255};
constexpr Fill Fill::Transparent = {0, 0, 0, 0};

enum class Direction {
    Right,
    Left,
    Down,
    Up
};

enum class AutoZoomMode {
    Fit,
    FitWidth,
    FitHeight,
    Original
};

 // This is sent to the shader as an int so the order matters
enum class InterpolationMode {
    Nearest,
    Linear,
    Smoothed,
    Cubic,
    SmartCubic
};

enum class TrimMode {
    None,
    PageCache
};

struct Mapping {
    control::Input input;
    control::Statement action;
};

struct WindowSettings {
    std::optional<geo::IVec> size;
    std::optional<bool> fullscreen;
    std::optional<FormatList> title;
};
struct LayoutSettings {
    std::optional<int32> spread_count;
    std::optional<Direction> spread_direction;
    std::optional<AutoZoomMode> auto_zoom_mode;
    std::optional<float> max_zoom;
    std::optional<float> min_zoomed_size;
    std::optional<bool> reset_zoom_on_page_turn;
    std::optional<geo::Vec> small_align;
    std::optional<geo::Vec> large_align;
};
struct RenderSettings {
    std::optional<InterpolationMode> interpolation_mode;
    std::optional<Fill> window_background;
    std::optional<Fill> transparency_background;
};
struct ControlSettings {
    std::optional<float> drag_speed;
};
struct FilesSettings {
    std::optional<SortMethod> sort;
    std::optional<std::set<AnyString>> page_extensions;
};
struct MemorySettings {
    std::optional<uint32> preload_ahead;
    std::optional<uint32> preload_behind;
    std::optional<double> page_cache_mb;
    std::optional<TrimMode> trim_when_minimized;
};

extern const Settings builtin_default_settings;

 // Using inheritance instead of containment because it makes using member
 // pointers much simpler.
struct Settings :
    WindowSettings, LayoutSettings, RenderSettings,
    ControlSettings, FilesSettings, MemorySettings
{
    UniqueArray<Mapping> mappings;
    const Settings* parent = &builtin_default_settings;

    template <class T, class Category>
    const T& get (std::optional<T> Category::*) const;

    const control::Statement* map_event (SDL_Event*) const;
};

template <class T, class Category>
const T& Settings::get (
    std::optional<T> Category::* setting
) const {
    auto generic = static_cast<std::optional<T> Settings::*>(setting);
    if (this->*generic) {
        return *(this->*generic);
    }
    else return parent->get(setting);
}

} // namespace liv
