#ifndef tap_disable_tests
#include "../tap/tap.h"
#include "../uni/string.h"
#include "floating.h"

using namespace geo;

template <class T>
void test_type (String name) {
    using namespace tap;
     // defined
    ok(!defined(T(GNAN)), name + " defined(GNAN)");
    ok(defined(T(-GINF)), name + " defined(-GINF)");
    ok(defined(T(0)), name + " defined(0)");
    ok(defined(T(GINF)), name + " defined(GINF)");
     // finite
    ok(!finite(T(GNAN)), name + " finite(GNAN)");
    ok(!finite(T(-GINF)), name + " finite(-GINF)");
    ok(finite(std::numeric_limits<T>::lowest()), name + " finite(lowest)");
    ok(finite(std::numeric_limits<T>::max()), name + " finite(max)");
    ok(!finite(T(GINF)), name + " finite(GINF)");
     // exact_eq
    ok(exact_eq(T(GNAN), T(GNAN)), name + " exact_eq(GNAN, GNAN)");
    ok(!exact_eq(T(GNAN), T(0)), name + " exact_eq(GNAN, 0)");
    ok(!exact_eq(T(GNAN), T(GINF)), name + " exact_eq(GNAN, GINF)");
    ok(exact_eq(T(-0.0), T(-0.0)), name + " exact_eq(-0, -0)");
    ok(!exact_eq(T(-0.0), T(0)), name + " exact_eq(-0, 0)");
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
}

static tap::TestSet tests ("base/geo/floating", []{
    using namespace tap;

    test_type<float>("float");
    test_type<double>("double");
    done_testing();
});

#endif
