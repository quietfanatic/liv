#pragma once

#include "../ayu/common.h"
#include "../uni/common.h"

namespace glow {

void init ();

namespace X {
    struct GlowError : hacc::X::Error {
        using hacc::X::Error::Error;
    };
}

} // namespace glow
