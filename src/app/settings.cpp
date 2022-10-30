#include "settings.h"

#include "../base/hacc/haccable.h"
#include "../base/hacc/haccable-standard.h"

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
        attr("mappings", &Settings::mappings)
    )
)
