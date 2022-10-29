#include "settings.h"

#include "../base/hacc/haccable.h"

namespace app {

} using namespace app;

HACCABLE(app::Settings,
    attrs(
        attr("prev", &Settings::prev),
        attr("next", &Settings::next),
        attr("quit", &Settings::quit)
    )
)
