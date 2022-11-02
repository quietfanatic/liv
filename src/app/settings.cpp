#include "settings.h"

#include "../base/ayu/describe.h"
#include "../base/ayu/describe-standard.h"

namespace app {

} using namespace app;

AYU_DESCRIBE(app::FitMode,
    values(
        value("fit", FIT),
        value("stretch", STRETCH),
        value("manual", MANUAL)
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
        attr("fit_mode", &Settings::Page::fit_mode, optional)
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
