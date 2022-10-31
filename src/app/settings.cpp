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

AYU_DESCRIBE(app::View,
    attrs(
        attr("fit_mode", &View::fit_mode, optional),
        attr("zoom", &View::zoom, optional),
        attr("offset", &View::offset, optional),
        attr("fullscreen", &View::fullscreen, optional)
    )
)

AYU_DESCRIBE(app::Mapping,
    elems(
        elem(&Mapping::input),
        elem(&Mapping::action)
    )
)

AYU_DESCRIBE(app::Settings,
    attrs(
        attr("default_view", &Settings::default_view),
        attr("mappings", &Settings::mappings)
    )
)
