// Some convenient text-related functions

#pragma once

#include "common.h"

namespace uni {

int natural_compare (OldStr a, OldStr b);
inline bool natural_lessthan (OldStr a, OldStr b) {
    return natural_compare(a, b) < 0;
}

} // namespace uni
