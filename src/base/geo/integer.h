// Functions involving integral types
// You probably don't want to include this directly, include scalar.h instead.

#pragma once

#include "common.h"

namespace geo {

///// PROPERTIES

template <Signed T>
CE std::make_unsigned_t<Widen<T>> length2 (T v) {
    return wide_multiply(v, v);
}

template <Signed T>
CE std::make_unsigned_t<T> length (T v) {
     // Do we need to special-case when v is the smallest negative integer,
     // which when negated equals itself?  No!  Turns out casting that number to
     // unsigned gives the right answer anyway.
    return v >= 0 ? v : -v;
}

///// MODIFIERS

template <Signed T>
CE T normalize (T v) { return v > 0 ? 1 : v < 0 ? -1 : v; }

template <Integral T>
CE T next_quantum (T v) { return v+1; }
template <class T>
CE T* next_quantum (T* v) { return v+1; }

CE int8 prev_quantum (int8 v) { return v-1; }
CE uint8 prev_quantum (uint8 v) { return v-1; }
CE int16 prev_quantum (int16 v) { return v-1; }
CE uint16 prev_quantum (uint16 v) { return v-1; }
CE int32 prev_quantum (int32 v) { return v-1; }
CE uint32 prev_quantum (uint32 v) { return v-1; }
CE int64 prev_quantum (int64 v) { return v-1; }
CE uint64 prev_quantum (uint64 v) { return v-1; }
template <class T>
CE T* prev_quantum (T* v) { return v-1; }

///// COMBINERS

template <Integral A, Integral B>
CE auto mod (A a, B b) { return a % b; }

 // rem is like mod, but always has the sign of the right side.
template <Signed A, Integral B>
CE auto rem (A a, B b) {
    if (a >= 0) return a % b;
    else return -a % -b;
}
template <Unsigned A, Integral B>
CE auto rem (A a, B b) { return a % b; }

template <Signed A, Signed B>
CE A align (A a, B b) {
    return b >= 0 ? length(a) : -length(a);
}

template <Integral A, Integral B>
CE std::common_type<A, B> lerp (A a, B b, double t) {
    return lerp(double(a), double(b), t);
}

 // This is stupid and I'm sure I'll regret it
template <class T>
CE T* lerp (T* a, T* b, double t) {
    DA(t >= 0 && t <= 1);
    T* r = a + round((b - a) * t);
}

} // namespace geo
