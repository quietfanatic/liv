#include "settings.h"

#include "../base/ayu/describe.h"
#include "../base/ayu/resource.h"

namespace app {

const Settings builtin_default_settings = {
    {
        .auto_zoom_mode = FIT,
        .max_zoom = 32,
        .min_page_size = 16,
        .reset_zoom_on_page_turn = true,
        .small_align = geo::Vec{0.5, 0.5},
        .large_align = geo::Vec{0.5, 0.5},
        .interpolation_mode = SMART_CUBIC,
    },
    {
        .size = geo::IVec{720, 720},
        .fullscreen = false,
    },
    {
        .supported_extensions = std::vector<String>{
            "bmp", "gif", "jfif", "jpe", "jpeg", "jpg", "png",
            "tif", "tiff", "xbm", "xpm", "webp",
        },
    },
    {
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

AYU_DESCRIBE(app::AutoZoomMode,
    values(
        value("fit", FIT),
        value("fit_width", FIT_WIDTH),
        value("fit_height", FIT_HEIGHT),
        value("fill", FILL),
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

AYU_DESCRIBE(app::Mapping,
    elems(
        elem(&Mapping::input),
        elem(&Mapping::action)
    )
)

AYU_DESCRIBE(app::PageSettings,
    attrs(
        attr("auto_zoom_mode", &PageSettings::auto_zoom_mode, optional),
        attr("reset_zoom_on_page_turn", &PageSettings::reset_zoom_on_page_turn, optional),
        attr("max_zoom", &PageSettings::max_zoom, optional),
        attr("min_page_size", &PageSettings::min_page_size, optional),
        attr("small_align", &PageSettings::small_align, optional),
        attr("large_align", &PageSettings::large_align, optional),
        attr("interpolation_mode", &PageSettings::interpolation_mode, optional)
    )
)

AYU_DESCRIBE(app::WindowSettings,
    attrs(
        attr("size", &WindowSettings::size, optional),
        attr("fullscreen", &WindowSettings::fullscreen, optional)
    )
)

AYU_DESCRIBE(app::FilesSettings,
    attrs(
        attr("supported_extensions", &FilesSettings::supported_extensions, optional)
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
        attr("page", base<PageSettings>(), optional),
        attr("window", base<WindowSettings>(), optional),
        attr("files", base<FilesSettings>(), optional),
        attr("memory", base<MemorySettings>(), optional),
        attr("mappings", &Settings::mappings)
    )
)
