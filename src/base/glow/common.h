#pragma once

#include "../ayu/common.h"
#include "../uni/common.h"

namespace glow {

void init ();

namespace X {
    struct GlowError : ayu::X::Error {
        using ayu::X::Error::Error;
    };
}

} // namespace glow
