#include "settings.h"

#include "../base/ayu/describe.h"
#include "../base/ayu/resource.h"

namespace app {

const Settings builtin_default_settings = {
    WindowSettings{
        .size = geo::IVec{720, 720},
        .fullscreen = false,
        .window_background = BLACK,
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
        .supported_extensions = std::set<String>{
            "bmp", "gif", "jfif", "jpe", "jpeg", "jpg", "png",
            "tif", "tiff", "xbm", "xpm", "webp",
        },
    },
    MemorySettings{
        .preload_ahead = 1,
        .preload_behind = 1,
        .page_cache_mb = 200
    },
    { }
};

const Settings* res_default_settings;

void init_settings () {
    res_default_settings = ayu::Resource("res:/app/settings-default.ayu").ref();
}

} using namespace app;

AYU_DESCRIBE(app::Fill,
    values(
        value("black", BLACK),
        value("white", WHITE),
        value("transparent", TRANSPARENT)
    ),
    delegate(base<glow::RGBA8>())
)

AYU_DESCRIBE(app::AutoZoomMode,
    values(
        value("fit", FIT),
        value("fit_width", FIT_WIDTH),
        value("fit_height", FIT_HEIGHT),
        value("original", ORIGINAL)
    )
)

AYU_DESCRIBE(app::InterpolationMode,
    values(
        value("nearest", NEAREST),
        value("linear", LINEAR),
        value("smoothed", SMOOTHED),
        value("cubic", CUBIC),
        value("smart_cubic", SMART_CUBIC)
    )
)

AYU_DESCRIBE(app::SpreadDirection,
    values(
        value("right", RIGHT),
        value("left", LEFT),
        value("down", DOWN),
        value("up", UP)
    )
)

AYU_DESCRIBE(app::Mapping,
    elems(
        elem(&Mapping::input),
        elem(&Mapping::action)
    )
)

AYU_DESCRIBE(app::LayoutSettings,
    attrs(
        attr("spread_count", &LayoutSettings::spread_count, optional),
        attr("spread_direction", &LayoutSettings::spread_direction, optional),
        attr("auto_zoom_mode", &LayoutSettings::auto_zoom_mode, optional),
        attr("reset_zoom_on_page_turn", &LayoutSettings::reset_zoom_on_page_turn, optional),
        attr("max_zoom", &LayoutSettings::max_zoom, optional),
        attr("min_zoomed_size", &LayoutSettings::min_zoomed_size, optional),
        attr("small_align", &LayoutSettings::small_align, optional),
        attr("large_align", &LayoutSettings::large_align, optional)
    )
)
AYU_DESCRIBE(app::PageSettings,
    attrs(
        attr("interpolation_mode", &PageSettings::interpolation_mode, optional)
    )
)

AYU_DESCRIBE(app::WindowSettings,
    attrs(
        attr("size", &WindowSettings::size, optional),
        attr("fullscreen", &WindowSettings::fullscreen, optional),
        attr("window_background", &WindowSettings::window_background, optional)
    )
)

AYU_DESCRIBE(app::FilesSettings,
    attrs(
        attr("supported_extensions", &FilesSettings::supported_extensions, optional)
    )
)

AYU_DESCRIBE(app::ControlSettings,
    attrs(
        attr("drag_speed", &ControlSettings::drag_speed, optional)
    )
)

AYU_DESCRIBE(app::MemorySettings,
    attrs(
        attr("preload_ahead", &MemorySettings::preload_ahead, optional),
        attr("preload_behind", &MemorySettings::preload_behind, optional),
        attr("page_cache_mb", &MemorySettings::page_cache_mb, optional)
    )
)

AYU_DESCRIBE(app::Settings,
    attrs(
        attr("window", base<WindowSettings>(), optional),
        attr("layout", base<LayoutSettings>(), optional),
        attr("page", base<PageSettings>(), optional),
        attr("control", base<ControlSettings>(), optional),
        attr("files", base<FilesSettings>(), optional),
        attr("memory", base<MemorySettings>(), optional),
        attr("mappings", &Settings::mappings)
    )
)
