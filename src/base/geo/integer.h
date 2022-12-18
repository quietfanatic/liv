// Functions involving integral types
// You probably don't want to include this directly, include scalar.h instead.

#pragma once

#include "common.h"
#include "type_traits.h"

namespace geo {

///// PROPERTIES

 // AKA sqr
template <SignedIntegral T>
CE MakeUnsigned<Widen<T>> length2 (T v) {
    return widen(v) * widen(v);
}

 // AKA sign
template <SignedIntegral T>
CE MakeUnsigned<T> length (T v) {
     // Do we need to special-case when v is the smallest negative integer,
     // which when negated equals itself?  No!  Turns out casting that number to
     // unsigned gives the right answer anyway.
    return v >= 0 ? v : -v;
}

///// MODIFIERS

template <SignedIntegral T>
CE T normalize (T v) {
    return v > 0 ? 1 : v < 0 ? -1 : v;
}

template <Integral T>
CE T next_quantum (T v) { return v+1; }
template <Pointing P>
CE P next_quantum (P v) { return v+1; }

template <Integral T>
CE T prev_quantum (T v) { return v-1; }
template <Pointing P>
CE P prev_quantum (P v) { return v-1; }

///// COMBINERS

template <Integral A, Integral B>
CE auto mod (A a, B b) { return a % b; }

 // rem is like mod, but always has the sign of the right side.
template <SignedIntegral A, Integral B>
CE auto rem (A a, B b) {
    if (a >= 0) return a % b;
    else return -a % -b;
}
template <UnsignedIntegral A, Integral B>
CE auto rem (A a, B b) { return a % b; }

 // AKA copysign
template <SignedIntegral A, SignedIntegral B>
CE A align (A a, B b) {
    return b >= 0 ? length(a) : -length(a);
}

 // This algorithm is slightly better for integers than a(1-t) * bt
template <Integral A, Integral B, Fractional T>
CE std::common_type<A, B> lerp (A a, B b, T t) {
    return a + round((b - a) * t);
}

 // Lerping pointers!  This is stupid and I'm sure I'll regret it.
template <Pointing P, Fractional T>
CE P lerp (P a, P b, T t) {
     // For safety, ensure we don't go outside the given range.  Note that if we
     // are given a standard begin-end pair (where end cannot be dereferenced),
     // the undereferencable end will be returned if t == 1.
    DA(t >= T(0) && t <= T(1));
    return a + round((b - a) * t);
}

} // namespace geo
