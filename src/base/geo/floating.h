// Utilities for dealing with floating point numbers.
// You probably don't want to include this directly, include scalar.h instead.

#pragma once

#include <bit>
#include <limits>
#include "common.h"
#include "values.h"
#include "type_traits.h"

namespace geo {

template <Floating T>
CE bool exact_eq (T a, T b) {
    return std::bit_cast<SameSizeInt<T>>(a) == std::bit_cast<SameSizeInt<T>>(b);
}

///// PROPERTIES

template <Floating T>
CE bool defined (T a) {
    return a == a;
}

template <Floating T>
CE bool finite (T a) {
     // A full exponent mask means the number is not finite.  This ends up being
     // much quicker than comparing against GNAN, GINF, and -GINF individually,
     // because the optimizer basically doesn't touch floating point
     // expressions.
    auto rep = std::bit_cast<SameSizeInt<T>>(a);
    auto mask = TypeTraits<T>::EXPONENT_MASK;
    return (rep & mask) != mask;
}
static_assert(!finite(float(GINF)));
static_assert(!finite(float(-GINF)));
static_assert(!finite(float(GNAN)));
static_assert(finite(std::numeric_limits<float>::max()));
static_assert(finite(std::numeric_limits<float>::lowest()));
static_assert(!finite(double(GINF)));
static_assert(!finite(double(-GINF)));
static_assert(!finite(double(GNAN)));
static_assert(finite(std::numeric_limits<double>::max()));
static_assert(finite(std::numeric_limits<double>::lowest()));

template <Floating T>
CE T length2 (T v) { return v * v; }
 // Okay, I admit, I just wanted a constexpr abs
template <Floating T>
CE T length (T v) { return v >= 0 ? v : -v; }

///// MODIFIERS

 // Round toward 0.  These will debug-assert if the number is NAN or can't fit
 // in an integer of the same size.
template <Floating T>
CE SameSizeInt<T> trunc (T a) {
    DA(a >= SameSizeInt<T>(-GINF) && a <= SameSizeInt<T>(GINF));
    return SameSizeInt<T>(a);
}

 // Round towards nearest integer.  0.5 => 1, -0.5 => -1
template <Floating T>
CE SameSizeInt<T> round (T a) {
    if (a >= 0) return trunc(a + T(0.5));
    else return trunc(a - T(0.5));
}

 // Round toward negative infinity
template <Floating T>
CE SameSizeInt<T> floor (T a) {
    if (a >= 0) return trunc(a);
    else return SameSizeInt<T>(-GINF) - trunc(SameSizeInt<T>(-GINF) - a);
}

template <Floating T>
CE SameSizeInt<T> ceil (T a) {
    if (a > 0) return SameSizeInt<T>(GINF) - trunc(SameSizeInt<T>(GINF) - a);
    else return trunc(a);
}

 // Get next larger representable value.
 // guarantees next_quantum(v) > v unless v is NAN or INF.
template <Floating T>
CE T next_quantum (T v) {
    if (!finite(v)) {
        if (exact_eq(v, TypeTraits<T>::MINUS_INF)) {
            return TypeTraits<T>::MINUS_HUGE;
        }
        else return v;
    }
    else if (exact_eq(v, TypeTraits<T>::MINUS_ZERO)) {
         // -0 == 0, so skip over 0
        return TypeTraits<T>::PLUS_TINY;
    }
    else {
        auto rep = std::bit_cast<SameSizeInt<T>>(v);
        if (rep & TypeTraits<T>::SIGN_BIT) {
            return std::bit_cast<T>(rep - 1);
        }
        else return std::bit_cast<T>(rep + 1);
    }
}

 // Get next smaller representable value.
 // guarantees prev_quantum(v) < v unless v is NAN or -INF.
template <Floating T>
CE T prev_quantum (T v) {
    if (!finite(v)) {
        if (exact_eq(v, TypeTraits<T>::PLUS_INF)) {
            return TypeTraits<T>::PLUS_HUGE;
        }
        else return v;
    }
    else if (exact_eq(v, TypeTraits<T>::PLUS_ZERO)) {
         // -0 == 0, so skip over -0
        return TypeTraits<T>::MINUS_TINY;
    }
    else {
        auto rep = std::bit_cast<SameSizeInt<T>>(v);
        if (rep & TypeTraits<T>::SIGN_BIT) {
            return std::bit_cast<T>(rep + 1);
        }
        else return std::bit_cast<T>(rep - 1);
    }
}

 // AKA sign for scalars
 // (Can't use (v > 0) - (v < 0) because it converts NAN to 0)
template <Floating T>
CE T normalize (T v) {
    return v > 0 ? 1 : v < 0 ? -1 : v;
}

///// COMBINERS

 // These will not work if a / b is inordinately large.
template <Floating A, Floating B>
CE A mod (A a, B b) {
    A ratio = a / b;
    if (ratio >= SameSizeInt<A>(-GINF) && ratio <= SameSizeInt<A>(GINF)) {
        return a - trunc(ratio) * b;
    }
    else return GNAN;
}

 // Like mod but sign is always sign of b
template <Floating A, Floating B>
CE A rem (A a, B b) {
    A ratio = a / b;
    if (ratio >= SameSizeInt<A>(-GINF) && ratio <= SameSizeInt<A>(GINF)) {
        return a - floor(ratio) * b;
    }
    else return GNAN;
}

 // AKA copysign
template <Floating A, Floating B>
CE A align (A a, B b) {
    return b >= 0 ? length(a) : -length(a);
}

 // This is the standard lerping formula.
template <Floating A, Floating B, Fractional T>
CE auto lerp (A a, B b, T t) {
    return (1-t)*a + t*b;
}

} // namespace geo
