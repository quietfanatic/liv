// Implements various utilities for scalar math, such as symbolic INC and NAN,
// constexpr rounding functions, and vector-like scalar functions.

#pragma once

#include <limits>
#include "../uni/common.h"

namespace geo {

///// SPECIAL VALUES

 // Represents not-a-number, or an undefined number.  Only representable by
 //  floating point types.
#undef NAN
struct NAN_t {
    CE operator float () const {
        return std::numeric_limits<float>::quiet_NaN();
    }
    CE NAN_t operator + () const { return *this; }
    CE NAN_t operator - () const { return *this; }
};
CE NAN_t NAN;
 // Represents infinity.  Don't cast this to signed integer types, because
 //  it'll always result in the minimum value even if it's positive INF.
#undef INF
struct INF_t {
    bool minus = false;
    CE operator float () const {
        return minus ? -std::numeric_limits<float>::infinity()
                     : std::numeric_limits<float>::infinity();
    }
    CE INF_t operator + () const { return *this; }
    CE INF_t operator - () const { return {!minus}; }
};
CE INF_t INF;

 // Represents the minimum or maximum value of whatever it's cast to.
 // TODO: merge this with INF_t
#undef MAX
struct MINMAX_t {
    bool minus;
    template <class T>
    CE operator T () const {
        return minus ? std::numeric_limits<T>::min()
                     : std::numeric_limits<T>::max();
    }
    CE MINMAX_t operator + () const { return *this; }
    CE MINMAX_t operator - () const { return {!minus}; }
};
CE MINMAX_t MIN {true};
CE MINMAX_t MAX {false};

#define SPECIAL_COMPARISON(Special, op) \
template <class T> \
CE bool operator op (Special a, T b) { \
    return T(a) op b; \
} \
template <class T> \
CE bool operator op (T a, Special b) { \
    return a op T(b); \
}
SPECIAL_COMPARISON(MINMAX_t, ==)
SPECIAL_COMPARISON(MINMAX_t, !=)
SPECIAL_COMPARISON(MINMAX_t, <)
SPECIAL_COMPARISON(MINMAX_t, <=)
SPECIAL_COMPARISON(MINMAX_t, >=)
SPECIAL_COMPARISON(MINMAX_t, >)
#undef SPECIAL_COMPARISON

///// NORMAL SCALAR FUNCTIONS

CE bool defined (float a) { return a == a; }
CE bool defined (double a) { return a == a; }

CE bool finite (float a) { return a == a && a != INF && a != -INF; }
CE bool finite (double a) { return a == a && a != INF && a != -INF; }

 // min and max propagate NANs and prefer a if equal
template <class T>
CE T min (T a, T b) {
    return a != a ? a
         : b != b ? b
         : a <= b ? a : b;
}
template <class T>
CE T max (T a, T b) {
    return a != a ? a
         : b != b ? b
         : a >= b ? a : b;
}

template <class T, class Low, class High>
CE T clamp (T a, Low low, High high) {
    return a != a ? a
         : a < low ? T(low)
         : a > high ? T(high)
         : a;
}

///// Fast constexpr rounding functions

 // Round toward 0
CE int32 trunc (float a) {
    DA(a >= int32(MIN) && a <= int32(MAX));
    return int32(a);
}
CE int64 trunc (double a) {
    DA(a >= int32(MIN) && a <= int32(MAX));
    return int64(a);
}

 // 0.5 => 1, -0.5 => -1
CE int32 round (float a) {
    if (a >= 0) return trunc(a + 0.5f);
    else return trunc(a - 0.5f);
}
CE int64 round (double a) {
    if (a >= 0) return trunc(a + 0.5);
    else return trunc(a - 0.5);
}

CE int32 floor (float a) {
    if (a >= 0) return trunc(a);
    else return int32(MIN) - trunc(int32(MIN) - a);
}
CE int64 floor (double a) {
    if (a >= 0) return int64(a);
    else return int64(MIN) - trunc(int64(MIN) - a);
}

CE int32 ceil (float a) {
    if (a > 0) return int32(MAX) - trunc(int32(MAX) - a);
    else return trunc(a);
}
CE int64 ceil (double a) {
    if (a > 0) return int64(MAX) - trunc(int64(MAX) - a);
    else return trunc(a);
}

///// Fast constexpr modulo and remainder
 // These will not work if a / b is inordinately large.
CE float mod (float a, float b) {
    float ratio = a / b;
    if (ratio >= int32(MIN) && ratio <= int32(MAX)) {
        return a - trunc(ratio) * b;
    }
    else return NAN;
}
CE double mod (double a, double b) {
    double ratio = a / b;
    if (ratio >= int32(MIN) && ratio <= int32(MAX)) {
        return a - trunc(ratio) * b;
    }
    else return NAN;
}
CE int32 mod (int32 a, int32 b) { return a % b; }
CE int64 mod (int64 a, int64 b) { return a % b; }

 // Like mod but sign is always sign of b
CE float rem (float a, float b) {
    float ratio = a / b;
    if (ratio >= int32(MIN) && ratio <= int32(MAX)) {
        return a - floor(ratio) * b;
    }
    else return NAN;
}
CE double rem (double a, double b) {
    double ratio = a / b;
    if (ratio >= int32(MIN) && ratio <= int32(MAX)) {
        return a - floor(ratio) * b;
    }
    else return NAN;
}
CE int32 rem (int32 a, int32 b) {
    if (a >= 0) return a % b;
    else return -a % -b;
}
CE int64 rem (int64 a, int64 b) {
    if (a >= 0) return a % b;
    else return -a % -b;
}

///// VECTOR-LIKE FUNCTIONS FOR SCALARS

CE float length2 (float v) { return v * v; }
CE double length2 (double v) { return v * v; }
CE int32 length2 (int32 v) { return v * v; }
CE int64 length2 (int64 v) { return v * v; }
 // Okay, I admit, I just wanted a constexpr abs
CE float length (float v) { return v >= 0 ? v : -v; }
CE double length (double v) { return v >= 0 ? v : -v; }
CE int32 length (int32 v) { return v >= 0 ? v : -v; }
CE int64 length (int64 v) { return v >= 0 ? v : -v; }

 // AKA sign for scalars
 // (Can't use (v > 0) - (v < 0) because it converts NAN to 0)
CE float normalize (float v) { return v > 0 ? 1 : v < 0 ? -1 : v; }
CE double normalize (double v) { return v > 0 ? 1 : v < 0 ? -1 : v; }
CE int32 normalize (int32 v) { return v > 0 ? 1 : v < 0 ? -1 : v; }
CE int64 normalize (int64 v) { return v > 0 ? 1 : v < 0 ? -1 : v; }

 // AKA copysign for scalars
CE float align (float a, float b) {
    return b >= 0 ? length(a) : -length(a);
}
CE double align (double a, double b) {
    return b >= 0 ? length(a) : -length(a);
}
CE int32 align (int32 a, int32 b) {
    return b >= 0 ? length(a) : -length(a);
}
CE int64 align (int64 a, int64 b) {
    return b >= 0 ? length(a) : -length(a);
}

CE float lerp (float a, float b, float t) {
    return t*a + (1-t)*b;
}
CE double lerp (double a, double b, double t) {
    return t*a + (1-t)*b;
}
CE int32 lerp (int32 a, int32 b, double t) {
    return int32(lerp(double(a), double(b), t));
}
CE int64 lerp (int64 a, int64 b, double t) {
    return int64(lerp(double(a), double(b), t));
}

} // namespace geo
