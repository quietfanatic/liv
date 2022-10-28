#include "test-environment.h"

#include "common.h"

namespace glow {

TestEnvironment::TestEnvironment (geo::IVec size) :
    size(size),
    window{
        .title = "Test window",
        .size = size,
        .hidden = true
    }
{
    window.open();
    glow::init();
}
TestEnvironment::~TestEnvironment () { }

} // namespace glow
