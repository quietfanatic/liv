// Some convenient text-related functions

#pragma once

#include "common.h"
#include "strings.h"

namespace uni {

 // Does a comparison for a "natural sort", where numbers within the string are
 // sorted by their numeric value regardless of how many digits they are.  The
 // behavior of corner cases may change in future updates.
int natural_compare (Str a, Str b);
inline bool natural_lessthan (Str a, Str b) {
    return natural_compare(a, b) < 0;
}

} // namespace uni
