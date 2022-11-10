#include "test-environment.h"

#include "common.h"
#include "gl.h"
#include "image.h"

namespace glow {

TestEnvironment::TestEnvironment (geo::IVec size) :
    size(size),
    window("Test window", size, wind::GLAttributes{.alpha = 8})
{
    glow::init();
}

TestEnvironment::~TestEnvironment () { }

Image TestEnvironment::read_pixels () {
    Image r (size);
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, r.pixels);
    return r;
}

} // namespace glow
