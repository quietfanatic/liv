#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"
#include "vec.h"

using namespace geo;

static tap::TestSet tests ("base/geo/vec", []{
    using namespace tap;
    IVec foo (4, 5);
    auto [a, b] = foo;
    is(a, 4);
    is(b, 5);
    done_testing();
});

#endif
