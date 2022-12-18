#ifndef tap_disable_tests
#include "../tap/tap.h"
#include "floating.h"

using namespace geo;

static tap::TestSet tests ("base/geo/floating", []{
    using namespace tap;
    auto test_floats = [](auto v, String name){
        using T = decltype(v);
         // exact_eq
        ok(exact_eq(T(GNAN), T(GNAN)), name + " exact_eq(GNAN, GNAN)");
        ok(!exact_eq(T(GNAN), T(0)), name + " !exact_eq(GNAN, 0)");
        ok(!exact_eq(T(GNAN), T(GINF)), name + " !exact_eq(GNAN, GINF)");
        ok(exact_eq(T(-0.0), T(-0.0)), name + " exact_eq(-0, -0)");
        ok(!exact_eq(T(-0.0), T(0)), name + " !exact_eq(-0, 0)");
         // root2
        ok(!defined(root2(T(GNAN))), name + " root2(GNAN)");
        ok(!defined(root2(T(-GINF))), name + " root2(-GINF)");
        ok(!defined(root2(T(-1))), name + " root2(-1)");
        ok(exact_eq(root2(T(-0.0)), T(-0.0)), name + " root2(-0)");
        ok(exact_eq(root2(T(0)), T(0)), name + " root2(0)");
        is(root2(T(1)), T(1), name + " root2(1)");
        is(root2(T(4)), T(2), name + " root2(4)");
        is(root2(T(GINF)), T(GINF), name + " root2(GINF)");
         // slow_root2
        ok(!defined(slow_root2(T(GNAN))), name + " slow_root2(GNAN)");
        ok(!defined(slow_root2(T(-GINF))), name + " slow_root2(-GINF)");
        ok(!defined(slow_root2(T(-1))), name + " slow_root2(-1)");
        ok(exact_eq(slow_root2(T(-0.0)), T(-0.0)), name + " slow_root2(-0)");
        ok(exact_eq(slow_root2(T(0)), T(0)), name + " slow_root2(0)");
        is(slow_root2(T(1)), T(1), name + " slow_root2(1)");
        is(slow_root2(T(4)), T(2), name + " slow_root2(4)");
        is(slow_root2(T(GINF)), T(GINF), name + " slow_root2(GINF)");
    };
    test_floats(float(0), "float");
    test_floats(double(0), "double");
    done_testing();
});

#endif
