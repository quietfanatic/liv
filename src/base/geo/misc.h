// This header contains functions that work on multiple data types, so they
// don't really belong in any specific type's header.

#pragma once

#include "../uni/macros.h"
#include "scalar.h"

namespace geo {

 // These work on anything that has length (or length2) and can be subtracted.
template <class TA, class TB>
CE auto distance2 (const TA& a, const TB& b) {
    return length2(b - a);
}

template <class TA, class TB>
CE auto distance (const TA& a, const TB& b) {
    return length(b - a);
}

} // namespace geo
