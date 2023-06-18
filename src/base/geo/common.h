#pragma once

#include "../uni/common.h"

namespace geo {
using namespace uni;

#if defined(min) || defined(max)
#warning "Uh oh, somebody defined min or max as a macro.  If you're using windows headers, you need to #define NOMINMAX first."
#endif
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

} // namespace geo
