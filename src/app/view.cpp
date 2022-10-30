#include "view.h"

#include "../base/hacc/haccable-standard.h"

namespace app {

} using namespace app;

HACCABLE(app::FitMode,
    values(
        value("fit", FIT),
        value("stretch", STRETCH),
        value("manual", MANUAL)
    )
)

HACCABLE(app::View,
    attrs(
        attr("fit_mode", &View::fit_mode),
        attr("zoom", &View::zoom)
    )
)
