// Functions involving integral types
// You probably don't want to include this directly, include scalar.h instead.

#pragma once

#include "common.h"

namespace geo {

///// PROPERTIES

CE uint16 length2 (int8 v) { return wide_multiply(v, v); }
CE uint32 length2 (int16 v) { return wide_multiply(v, v); }
CE uint64 length2 (int32 v) { return wide_multiply(v, v); }
CE uint64 length2 (int64 v) { return uint64(v) * uint64(v); }

CE uint8 length (int8 v) { return v >= 0 ? v : -v; }
CE uint16 length (int16 v) { return v >= 0 ? v : -v; }
CE uint32 length (int32 v) { return v >= 0 ? v : -v; }
CE uint64 length (int64 v) { return v >= 0 ? v : -v; }

///// MODIFIERS

CE int8 normalize (int8 v) { return v > 0 ? 1 : v < 0 ? -1 : v; }
CE int16 normalize (int16 v) { return v > 0 ? 1 : v < 0 ? -1 : v; }
CE int32 normalize (int32 v) { return v > 0 ? 1 : v < 0 ? -1 : v; }
CE int64 normalize (int64 v) { return v > 0 ? 1 : v < 0 ? -1 : v; }

CE int8 next_quantum (int8 v) { return v+1; }
CE uint8 next_quantum (uint8 v) { return v+1; }
CE int16 next_quantum (int16 v) { return v+1; }
CE uint16 next_quantum (uint16 v) { return v+1; }
CE int32 next_quantum (int32 v) { return v+1; }
CE uint32 next_quantum (uint32 v) { return v+1; }
CE int64 next_quantum (int64 v) { return v+1; }
CE uint64 next_quantum (uint64 v) { return v+1; }
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

CE int8 mod (int8 a, int8 b) { return a % b; }
CE uint8 mod (uint8 a, uint8 b) { return a % b; }
CE int16 mod (int16 a, int16 b) { return a % b; }
CE uint16 mod (uint16 a, uint16 b) { return a % b; }
CE int32 mod (int32 a, int32 b) { return a % b; }
CE uint32 mod (uint32 a, uint32 b) { return a % b; }
CE int64 mod (int64 a, int64 b) { return a % b; }
CE uint64 mod (uint64 a, uint64 b) { return a % b; }

CE int8 rem (int8 a, int8 b) {
    if (a >= 0) return a % b;
    else return -a % -b;
}
CE uint8 rem (uint8 a, uint8 b) {
    return a % b;
}
CE int16 rem (int16 a, int16 b) {
    if (a >= 0) return a % b;
    else return -a % -b;
}
CE uint16 rem (uint16 a, uint16 b) {
    return a % b;
}
CE int32 rem (int32 a, int32 b) {
    if (a >= 0) return a % b;
    else return -a % -b;
}
CE uint32 rem (uint32 a, uint32 b) {
    return a % b;
}
CE int64 rem (int64 a, int64 b) {
    if (a >= 0) return a % b;
    else return -a % -b;
}
CE uint64 rem (uint64 a, uint64 b) {
    return a % b;
}

 // TODO: int16 and int8
CE int32 align (int32 a, int32 b) {
    return b >= 0 ? length(a) : -length(a);
}
CE int64 align (int64 a, int64 b) {
    return b >= 0 ? length(a) : -length(a);
}

CE int32 lerp (int32 a, int32 b, double t) {
    return int32(lerp(double(a), double(b), t));
}
CE int64 lerp (int64 a, int64 b, double t) {
    return int64(lerp(double(a), double(b), t));
}
 // This is stupid and I'm sure I'll regret it
template <class T>
CE T* lerp (T* a, T* b, double t) {
    DA(t >= 0 && t <= 1);
    T* r = a + isize((b - a) * t);
}

} // namespace geo
