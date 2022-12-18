 // Utilities involving scalar types (either floating or integers).

#pragma once

#include "common.h"
#include "floating.h"
#include "integer.h"
#include "values.h"

namespace geo {

 // min and max propagate NANs and prefer left side if equal
template <class T>
CE auto min (T a) {
    return a;
}
template <class A, class B, class... Ts> requires (
    requires (A a, B b) { a <= b; }
)
CE auto min (A a, B b, Ts&&... rest) {
    return a != a ? a
         : a <= b ? a
         : min(b, std::forward<Ts>(rest)...);
}
template <class T>
CE auto max (T a) {
    return a;
}
template <class A, class B, class... Ts> requires (
    requires (A a, B b) { a <= b; }
)
CE auto max (A a, B b, Ts&&... rest) {
    return a != a ? a
         : a >= b ? a
         : max(b, std::forward<Ts>(rest)...);
}

 // clamp returns NAN if any argument is NAN.
template <class T, class Low, class High>
CE T clamp (T a, Low low, High high) {
    if (a != a) return a;
    if (a >= low) {
        if (a <= high) return a;
        else return T(high);
    }
    else return T(low);
}

 // These work on anything that has length (or length2) and can be subtracted.
template <class TA, class TB>
CE auto distance2 (TA a, TB b) {
    return length2(b - a);
}

template <class TA, class TB>
CE auto distance (TA a, TB b) {
    return length(b - a);
}

} // namespace geo
