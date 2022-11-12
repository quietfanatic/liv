#pragma once

#include <compare>
#include "common.h"

namespace uni {

int natural_compare (Str a, Str b);
inline bool natural_lessthan (Str a, Str b) {
    return natural_compare(a, b) < 0;
}

} // namespace uni
