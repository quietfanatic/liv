#include "settings.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/resources/resource.h"

namespace liv {

const Settings builtin_default_settings = {
    WindowSettings{
        .size = geo::IVec{720, 720},
        .fullscreen = false,
        .window_background = BLACK,
        .title = FormatList(
            FormatToken("["), FormatToken(FormatCommand::VisibleRange),
            FormatToken("/"), FormatToken(FormatCommand::PageCount),
            FormatToken("] "), FormatToken(FormatCommand::PageRelCwd),
            FormatToken(FormatCommand::IfZoomed, FormatList(
                FormatToken(" ("), FormatToken(FormatCommand::ZoomPercent),
                FormatToken("%)")
            ))
        ),
    },
    LayoutSettings{
        .spread_count = 1,
        .spread_direction = RIGHT,
        .auto_zoom_mode = FIT,
        .max_zoom = 32,
        .min_zoomed_size = 16,
        .reset_zoom_on_page_turn = true,
        .small_align = geo::Vec{0.5, 0.5},
        .large_align = geo::Vec{0.5, 0.5},
    },
    PageSettings{
        .interpolation_mode = SMART_CUBIC,
    },
    ControlSettings{
        .drag_speed = 1,
    },
    FilesSettings{
        .supported_extensions = std::set<AnyString>{
            "bmp", "gif", "jfif", "jpe", "jpeg", "jpg",
            "png", "tif", "tiff", "xbm", "xpm", "webp",
        },
    },
    MemorySettings{
        .preload_ahead = 1,
        .preload_behind = 1,
        .page_cache_mb = 200,
        .trim_when_minimized = TRIM_PAGE_CACHE
    },
    { }
};

const Settings* res_default_settings;

void init_settings () {
    if (!res_default_settings) {
        res_default_settings = ayu::Resource("res:/liv/settings-default.ayu").ref();
    }
}

} using namespace liv;

AYU_DESCRIBE(liv::Fill,
    values(
        value("black", BLACK),
        value("white", WHITE),
        value("transparent", TRANSPARENT)
    ),
    delegate(base<glow::RGBA8>())
)

AYU_DESCRIBE(liv::AutoZoomMode,
    values(
        value("fit", FIT),
        value("fit_width", FIT_WIDTH),
        value("fit_height", FIT_HEIGHT),
        value("original", ORIGINAL)
    )
)

AYU_DESCRIBE(liv::InterpolationMode,
    values(
        value("nearest", NEAREST),
        value("linear", LINEAR),
        value("smoothed", SMOOTHED),
        value("cubic", CUBIC),
        value("smart_cubic", SMART_CUBIC)
    )
)

AYU_DESCRIBE(liv::SpreadDirection,
    values(
        value("right", RIGHT),
        value("left", LEFT),
        value("down", DOWN),
        value("up", UP)
    )
)

AYU_DESCRIBE(liv::TrimMode,
    values(
        value("none", TRIM_NONE),
        value("page_cache", TRIM_PAGE_CACHE)
    )
)

AYU_DESCRIBE(liv::Mapping,
    elems(
        elem(&Mapping::input),
        elem(&Mapping::action)
    )
)

AYU_DESCRIBE(liv::LayoutSettings,
    attrs(
        attr("spread_count", &LayoutSettings::spread_count, collapse_optional),
        attr("spread_direction", &LayoutSettings::spread_direction, collapse_optional),
        attr("auto_zoom_mode", &LayoutSettings::auto_zoom_mode, collapse_optional),
        attr("reset_zoom_on_page_turn", &LayoutSettings::reset_zoom_on_page_turn, collapse_optional),
        attr("max_zoom", &LayoutSettings::max_zoom, collapse_optional),
        attr("min_zoomed_size", &LayoutSettings::min_zoomed_size, collapse_optional),
        attr("small_align", &LayoutSettings::small_align, collapse_optional),
        attr("large_align", &LayoutSettings::large_align, collapse_optional)
    )
)
AYU_DESCRIBE(liv::PageSettings,
    attrs(
        attr("interpolation_mode", &PageSettings::interpolation_mode, collapse_optional)
    )
)

AYU_DESCRIBE(liv::WindowSettings,
    attrs(
        attr("size", &WindowSettings::size, collapse_optional),
        attr("fullscreen", &WindowSettings::fullscreen, collapse_optional),
        attr("window_background", &WindowSettings::window_background, collapse_optional),
        attr("title", &WindowSettings::title, collapse_optional)
    )
)

AYU_DESCRIBE(liv::FilesSettings,
    attrs(
        attr("supported_extensions", &FilesSettings::supported_extensions, collapse_optional)
    )
)

AYU_DESCRIBE(liv::ControlSettings,
    attrs(
        attr("drag_speed", &ControlSettings::drag_speed, collapse_optional)
    )
)

AYU_DESCRIBE(liv::MemorySettings,
    attrs(
        attr("preload_ahead", &MemorySettings::preload_ahead, collapse_optional),
        attr("preload_behind", &MemorySettings::preload_behind, collapse_optional),
        attr("page_cache_mb", &MemorySettings::page_cache_mb, collapse_optional),
        attr("trim_when_minimized", &MemorySettings::trim_when_minimized, collapse_optional)
    )
)

AYU_DESCRIBE(liv::Settings,
    attrs(
        attr("window", base<WindowSettings>(), optional),
        attr("layout", base<LayoutSettings>(), optional),
        attr("page", base<PageSettings>(), optional),
        attr("control", base<ControlSettings>(), optional),
        attr("files", base<FilesSettings>(), optional),
        attr("memory", base<MemorySettings>(), optional),
        attr("mappings", &Settings::mappings, optional)
    )
)
