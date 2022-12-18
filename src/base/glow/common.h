#pragma once

#include "../ayu/common.h"
#include "../uni/common.h"
#include "../uni/macros.h"

namespace glow {
using namespace uni;

void init ();

struct GlowError : ayu::Error { };

} // namespace glow
