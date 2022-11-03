#include "settings.h"

#include "../base/ayu/describe.h"
#include "../base/ayu/describe-standard.h"

namespace app {

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
        value("cubic", CUBIC)
    )
)

AYU_DESCRIBE(app::Mapping,
    elems(
        elem(&Mapping::input),
        elem(&Mapping::action)
    )
)

AYU_DESCRIBE(app::Settings::Page,
    attrs(
        attr("auto_zoom_mode", &Settings::Page::auto_zoom_mode, optional),
        attr("reset_zoom_on_page_turn", &Settings::Page::reset_zoom_on_page_turn, optional),
        attr("max_zoom", &Settings::Page::max_zoom, optional),
        attr("min_page_size", &Settings::Page::min_page_size, optional),
        attr("small_align", &Settings::Page::small_align, optional),
        attr("large_align", &Settings::Page::large_align, optional),
        attr("interpolation_mode", &Settings::Page::interpolation_mode, optional)
    )
)

AYU_DESCRIBE(app::Settings::Window,
    attrs(
        attr("size", &Settings::Window::size, optional),
        attr("fullscreen", &Settings::Window::fullscreen, optional)
    )
)

AYU_DESCRIBE(app::Settings::Memory,
    attrs(
        attr("preload_ahead", &Settings::Memory::preload_ahead, optional),
        attr("preload_behind", &Settings::Memory::preload_behind, optional),
        attr("page_cache_mb", &Settings::Memory::page_cache_mb, optional)
    )
)

AYU_DESCRIBE(app::Settings,
    attrs(
        attr("page", &Settings::page, optional),
        attr("window", &Settings::window, optional),
        attr("memory", &Settings::memory, optional),
        attr("mappings", &Settings::mappings)
    )
)
