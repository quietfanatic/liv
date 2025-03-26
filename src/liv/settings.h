#pragma once

#include <optional>
#include "../dirt/control/command.h"
#include "../dirt/control/input.h"
#include "../dirt/geo/range.h"
#include "../dirt/geo/vec.h"
#include "../dirt/glow/colors.h"
#include "../dirt/iri/iri.h"
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

 // TODO: reorder
enum class Direction {
    Right,
    Left,
    Down,
    Up
};
constexpr Direction operator- (Direction dir) {
    return Direction{int(dir) ^ 1};
}

enum class AutoZoomMode {
    Fit,
    FitWidth,
    FitHeight,
    Original
};

enum class ResetOnSeek {
    None,
    Offset,
    Zoom
};

 // This is sent to the shader as an int so the order matters
enum class InterpolationMode {
    Nearest,
    Linear,
    Smoothed,
    Cubic,
    SmartCubic
};

struct ColorRange {
    geo::Range ranges [3];
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
     // For testing.  Don't show windows.  Note that some graphics drivers will
     // refuse to draw on hidden windows, so you can't test drawing when the
     // window is hidden.
    std::optional<bool> hidden;
     // For testing.  Disable workaround for false keypress on window focus.
    std::optional<bool> automated_input;
     // Not sure where to put this but here it is
    std::optional<AnyString> last_prompt_command;
};
struct LayoutSettings {
    std::optional<i32> spread_count;
    static constexpr i32 max_spread_count = 16;
    std::optional<Direction> spread_direction;
    std::optional<AutoZoomMode> auto_zoom_mode;
    std::optional<float> max_zoom;
    std::optional<float> min_zoomed_size;
    std::optional<ResetOnSeek> reset_on_seek;
    std::optional<geo::Vec> small_align;
    std::optional<geo::Vec> large_align;
    std::optional<float> scroll_margin;
    std::optional<Direction> orientation;
};
struct RenderSettings {
    std::optional<InterpolationMode> interpolation_mode;
    std::optional<Fill> window_background;
    std::optional<Fill> transparency_background;
    std::optional<ColorRange> color_range;
};
struct ControlSettings {
    std::optional<Vec> scroll_speed;
    std::optional<Vec> drag_speed;
};
struct FilesSettings {
    std::optional<SortMethod> sort;
     // Keep these in order
    std::optional<AnyArray<AnyString>> page_extensions;
};
struct MemorySettings {
    std::optional<u32> preload_ahead;
    std::optional<u32> preload_behind;
    std::optional<double> page_cache_mb;
    std::optional<TrimMode> trim_when_minimized;
};

extern const Settings builtin_default_settings;

struct Settings {
     // Parent is at the beginning in memory but at the end in ayu
    const Settings* parent = &builtin_default_settings;
    WindowSettings window;
    LayoutSettings layout;
    RenderSettings render;
    ControlSettings control;
    FilesSettings files;
    MemorySettings memory;
    UniqueArray<Mapping> mappings;

    void canonicalize ();

    template <class T, class Category>
    const T& get (std::optional<T> Category::*) const;

     // Anything set on the other settings will transferred to this one.  The
     // parent will also be transferred unless it is &builtin_default_settings.
    void merge (Settings&&);

    const control::Statement* map_input (control::Input) const;
};

template <class T, class Category>
const T& Settings::get (
    std::optional<T> Category::* setting
) const {
    if constexpr (requires { window.*setting; }) {
        if (window.*setting) return *(window.*setting);
    }
    else if constexpr (requires { layout.*setting; }) {
        if (layout.*setting) return *(layout.*setting);
    }
    else if constexpr (requires { render.*setting; }) {
        if (render.*setting) return *(render.*setting);
    }
    else if constexpr (requires { control.*setting; }) {
        if (control.*setting) return *(control.*setting);
    }
    else if constexpr (requires { files.*setting; }) {
        if (files.*setting) return *(files.*setting);
    }
    else if constexpr (requires { memory.*setting; }) {
        if (memory.*setting) return *(memory.*setting);
    }
    else static_assert((Category*)null, "Incompatible member pointer passed to Settings::get");
    return parent->get(setting);
}

} // namespace liv
