#include "mat.h"

#include "rect.h"

using namespace geo;

bool finite_a (const Rect& a) {
    return finite(a.l) && finite(a.b) && finite(a.r) && finite(a.t);
}

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"

static tap::TestSet tests ("base/geo/mat", []{
    using namespace tap;
    Mat3 t1 = add_row(
        add_column(
            Mat(Vec(2, 3)),
            Vec(0.1, 0.2)
        ), Vec3(0, 0, 3.5)
    );
    is(t1, Mat3(2, 0, 0,  0, 3, 0,  0.1, 0.2, 3.5), "add_row and add_column work");
    is(t1 * Vec3(4, 5, 1), Vec3(8.1, 15.2, 3.5), "Basic matrix multiplication works");
    done_testing();
});

#endif
