#include "settings.h"

#include "../base/ayu/describe.h"
#include "../base/ayu/describe-standard.h"

namespace app {

} using namespace app;

HACCABLE(app::Mapping,
    elems(
        elem(&Mapping::input),
        elem(&Mapping::action)
    )
)

HACCABLE(app::Settings,
    attrs(
        attr("default_view", &Settings::default_view),
        attr("mappings", &Settings::mappings)
    )
)
