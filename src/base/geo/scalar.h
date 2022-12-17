// Implements various utilities for scalar math, such as symbolic GINC and GNAN,
// constexpr rounding functions, and vector-like scalar functions.

#pragma once

#include <bit>
#include <limits>
#include "../uni/common.h"
#include "../uni/macros.h"

namespace geo {
using namespace uni;

///// SPECIAL GENERIC VALUES

 // Represents not-a-number, or an undefined number.  Only representable by
 // floating point types or things that contain them.
struct GNAN_t {
    CE operator float () const {
        return std::numeric_limits<float>::quiet_NaN();
    }
    CE GNAN_t operator + () const { return *this; }
    CE GNAN_t operator - () const { return *this; }
};
CE GNAN_t GNAN;

 // Represents the minimum or maximum value of whatever it's cast to.
struct GINF_t {
    bool minus = false;
    template <class T> requires (std::numeric_limits<T>::is_specialized)
    CE operator T () const {
        if constexpr (std::numeric_limits<T>::has_infinity) {
            return minus ? -std::numeric_limits<T>::infinity()
                         : std::numeric_limits<T>::infinity();
        }
        else {
            return minus ? std::numeric_limits<T>::lowest()
                         : std::numeric_limits<T>::max();
        }
    }
    CE GINF_t operator + () const { return *this; }
    CE GINF_t operator - () const { return {!minus}; }
};
CE GINF_t GINF;

#define GINF_COMPARISON(op) \
template <class T> \
CE bool operator op (GINF_t a, T b) { \
    return T(a) op b; \
} \
template <class T> \
CE bool operator op (T a, GINF_t b) { \
    return a op T(b); \
}
GINF_COMPARISON(==)
GINF_COMPARISON(!=)
GINF_COMPARISON(<)
GINF_COMPARISON(<=)
GINF_COMPARISON(>=)
GINF_COMPARISON(>)
#undef GINF_COMPARISON

///// SPECIAL CASE MULTIPLY THAT WIDENS int32 TO isize
 // This is used, for instance, by area()
template <class A, class B>
CE auto wide_multiply (const A& a, const B& b) {
    return a * b;
}
CE isize wide_multiply (int32 a, int32 b) {
    return isize(a) * isize(b);
}

///// NORMAL SCALAR FUNCTIONS

CE bool defined (float a) { return a == a; }
CE bool defined (double a) { return a == a; }

CE bool finite (float a) { return a == a && a != GINF && a != -GINF; }
CE bool finite (double a) { return a == a && a != GINF && a != -GINF; }

 // min and max propagate NANs and prefer left side if equal
template <class T>
CE auto min (T a) {
    return a;
}
template <class A, class B, class... Ts>
CE auto min (A a, B b, Ts&&... rest) {
    return a != a ? a
         : a <= b ? a
         : min(b, std::forward<Ts>(rest)...);
}
template <class T>
CE auto max (T a) {
    return a;
}
template <class A, class B, class... Ts>
CE auto max (A a, B b, Ts&&... rest) {
    return a != a ? a
         : a >= b ? a
         : max(b, std::forward<Ts>(rest)...);
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
    DA(a >= int32(-GINF) && a <= int32(GINF));
    return int32(a);
}
CE int64 trunc (double a) {
    DA(a >= int32(-GINF) && a <= int32(GINF));
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
    else return int32(-GINF) - trunc(int32(-GINF) - a);
}
CE int64 floor (double a) {
    if (a >= 0) return int64(a);
    else return int64(-GINF) - trunc(int64(-GINF) - a);
}

CE int32 ceil (float a) {
    if (a > 0) return int32(GINF) - trunc(int32(GINF) - a);
    else return trunc(a);
}
CE int64 ceil (double a) {
    if (a > 0) return int64(GINF) - trunc(int64(GINF) - a);
    else return trunc(a);
}

///// Fast constexpr modulo and remainder
 // These will not work if a / b is inordinately large.
CE float mod (float a, float b) {
    float ratio = a / b;
    if (ratio >= int32(-GINF) && ratio <= int32(GINF)) {
        return a - trunc(ratio) * b;
    }
    else return GNAN;
}
CE double mod (double a, double b) {
    double ratio = a / b;
    if (ratio >= int32(-GINF) && ratio <= int32(GINF)) {
        return a - trunc(ratio) * b;
    }
    else return GNAN;
}
CE int32 mod (int32 a, int32 b) { return a % b; }
CE int64 mod (int64 a, int64 b) { return a % b; }

 // Like mod but sign is always sign of b
CE float rem (float a, float b) {
    float ratio = a / b;
    if (ratio >= int32(-GINF) && ratio <= int32(GINF)) {
        return a - floor(ratio) * b;
    }
    else return GNAN;
}
CE double rem (double a, double b) {
    double ratio = a / b;
    if (ratio >= int32(-GINF) && ratio <= int32(GINF)) {
        return a - floor(ratio) * b;
    }
    else return GNAN;
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

 // Selecting preferred type for t passed to lerping functions.  Basically,
 // double for everything except float for floats.
template <class T>
struct PreferredLerperS { using type = double; };
template <>
struct PreferredLerperS<float> { using type = float; };
template <class T>
using PreferredLerper = PreferredLerperS<T>::type;

CE float lerp (float a, float b, float t) {
    return (1-t)*a + t*b;
}
CE double lerp (double a, double b, double t) {
    return (1-t)*a + t*b;
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

///// QUANTUM FUNCTIONS (no relation to QM)

 // Get the next representable value.  Does not change NANs or INFs.
 // Minus zero is treated the same as zero.  Includes subnormals.
CE int32 next_quantum (int32 v) { return v+1; }
CE int64 next_quantum (int64 v) { return v+1; }
CE float next_quantum (float v) {
    if (!finite(v)) return v;
    uint32 rep = std::bit_cast<uint32>(v);
    if (rep == 0x8000'0000) {
         // Treat minus zero as zero
        return std::bit_cast<float>(0x0000'0001);
    }
    else if (rep == 0x7f8f'ffff) {
         // Largest finite number
        return GINF;
    }
    else if (rep & 0x8000'0000) {
        return std::bit_cast<float>(rep - 1);
    }
    else {
        return std::bit_cast<float>(rep + 1);
    }
}
CE double next_quantum (double v) {
    if (!finite(v)) return v;
    uint64 rep = std::bit_cast<uint64>(v);
    if (rep == 0x8000'0000'0000'0000) {
         // Treat minus zero as zero
        return std::bit_cast<double>(uint64(0x0000'0000'0000'0001));
    }
    else if (rep == 0x7fef'ffff'ffff'ffff) {
         // Largest finite number
        return GINF;
    }
    else if (rep & 0x8000'0000'0000'0000) {
        return std::bit_cast<double>(rep - 1);
    }
    else {
        return std::bit_cast<double>(rep + 1);
    }
}
template <class T>
CE T* next_quantum (T* v) { return v+1; }

 // Get the previous representable value.
CE int32 prev_quantum (int32 v) { return v-1; }
CE int64 prev_quantum (int64 v) { return v-1; }
CE float prev_quantum (float v) {
    if (!finite(v)) return v;
    uint32 rep = std::bit_cast<uint32>(v);
    if (rep == 0x0000'0000) {
         // Skip minus zero
        return std::bit_cast<float>(0x8000'0001);
    }
    else if (rep == 0xff8f'ffff) {
         // Smallest finite number
        return -GINF;
    }
    else if (rep & 0x8000'0000) {
        return std::bit_cast<float>(rep + 1);
    }
    else {
        return std::bit_cast<float>(rep - 1);
    }
}
CE double prev_quantum (double v) {
    if (!finite(v)) return v;
    uint64 rep = std::bit_cast<uint64>(v);
    if (rep == 0x0000'0000'0000'0000) {
         // Skip minus zero
        return std::bit_cast<double>(0x8000'0000'0000'0001);
    }
    else if (rep == 0xffef'ffff'ffff'ffff) {
         // Smallest finite number
        return -GINF;
    }
    else if (rep & 0x8000'0000'0000'0000) {
        return std::bit_cast<double>(rep + 1);
    }
    else {
        return std::bit_cast<double>(rep - 1);
    }
}
template <class T>
CE T* prev_quantum (T* v) { return v-1; }

} // namespace geo
