#pragma once

#include "../ayu/common.h"
#include "../uni/common.h"
#include "../uni/macros.h"

namespace glow {
using namespace uni;

void init ();

namespace X {
    struct GlowError : ayu::X::Error {
        using ayu::X::Error::Error;
    };
}

} // namespace glow
