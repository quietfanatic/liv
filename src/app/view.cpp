#include "view.h"

#include "../base/hacc/haccable-standard.h"

namespace app {
using namespace geo;

Rect View::page_position (Vec page_size, Vec window_size) const {
    AA(page_size.x > 0 && page_size.y > 0);
    AA(window_size.x > 0 && window_size.y > 0);
    switch (fit_mode) {
        case FIT: {
             // Fit to screen
             // slope = 1/aspect
            float scale = slope(page_size) > slope(window_size)
                ? float(window_size.y) / page_size.y
                : float(window_size.x) / page_size.x;
            Rect page_rect = Rect({0, 0}, page_size * scale);
             // Center
            return page_rect + (window_size - page_rect.size()) / 2;
        }
        case STRETCH: {
            return Rect({0, 0}, window_size);
        }
        default:
        case MANUAL: {
             // TODO:
            return Rect({0, 0}, window_size);
        }
    }
}

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
        attr("fit_mode", &View::fit_mode, optional),
        attr("zoom", &View::zoom, optional),
        attr("offset", &View::offset, optional)
    )
)
