#pragma once

#include "../ayu/resource-scheme.h"
#include "../geo/vec.h"
#include "../wind/window.h"

namespace glow {
struct Image;

struct TestEnvironment {
    geo::IVec size;
    ayu::FileResourceScheme test_scheme;
    wind::Window window;
    TestEnvironment (geo::IVec size = {120, 120});
    ~TestEnvironment ();

    Image read_pixels ();
};

} // namespace glow
