// Using our own type traits system because std's type_traits is really sloppy
// about letting references through, and considers bools and chars to be
// integers, among other things.

#pragma once

#include <limits>
#include "common.h"

namespace geo {

template <class T>
struct TypeTraits {
    using Widen = T;
    static CE bool integral = false;
    static CE bool floating = false;
    static CE bool fractional = false;
    static CE bool is_signed = false;
};
template <> struct TypeTraits<int8> {
    using Widen = int16;
    using MakeUnsigned = uint8;
    static CE bool integral = true;
    static CE bool floating = false;
    static CE bool fractional = false;
    static CE bool is_signed = true;
};
template <> struct TypeTraits<uint8> {
    using Widen = uint16;
    using MakeSigned = int8;
    static CE bool integral = true;
    static CE bool floating = false;
    static CE bool fractional = false;
    static CE bool is_signed = false;
};
template <> struct TypeTraits<int16> {
    using Widen = int32;
    using MakeUnis_signed = uint16;
    static CE bool integral = true;
    static CE bool floating = false;
    static CE bool fractional = false;
    static CE bool is_signed = true;
};
template <> struct TypeTraits<uint16> {
    using Widen = uint32;
    using MakeSigned = int16;
    static CE bool integral = true;
    static CE bool floating = false;
    static CE bool fractional = false;
    static CE bool is_signed = false;
};
template <> struct TypeTraits<int32> {
    using Widen = int64;
    using MakeUnis_signed = uint32;
    static CE bool integral = true;
    static CE bool floating = false;
    static CE bool fractional = false;
    static CE bool is_signed = true;
};
template <> struct TypeTraits<uint32> {
    using Widen = uint64;
    using MakeSigned = int32;
    static CE bool integral = true;
    static CE bool floating = false;
    static CE bool fractional = false;
    static CE bool is_signed = false;
};
template <> struct TypeTraits<int64> {
    using Widen = int64;
    using MakeUnis_signed = int64;
    static CE bool integral = true;
    static CE bool floating = false;
    static CE bool fractional = false;
    static CE bool is_signed = true;
};
template <> struct TypeTraits<uint64> {
    using Widen = uint64;
    using MakeSigned = int64;
    static CE bool integral = true;
    static CE bool floating = false;
    static CE bool fractional = false;
    static CE bool is_signed = false;
};
template <> struct TypeTraits<float> {
    using Widen = float;
    using SameSizeInt = int32;
    static CE bool integral = false;
    static CE bool floating = true;
    static CE bool fractional = true;
    static CE bool is_signed = true;
     // A number of functions in this library assume standard floating point
     // layout.
    static CE uint32 SIGN_BIT = 0x8000'0000;
    static CE uint32 EXPONENT_MASK = 0x7f80'0000;
    static CE float MINUS_INF = std::bit_cast<float>(0xff80'0000);
    static CE float MINUS_HUGE = std::bit_cast<float>(0xff7f'ffff);
    static CE float MINUS_TINY = std::bit_cast<float>(0x8000'0001);
    static CE float MINUS_ZERO = std::bit_cast<float>(0x8000'0000);
    static CE float PLUS_ZERO = std::bit_cast<float>(0x0000'0000);
    static CE float PLUS_TINY = std::bit_cast<float>(0x0000'0001);
    static CE float PLUS_HUGE = std::bit_cast<float>(0x7f7f'ffff);
    static CE float PLUS_INF = std::bit_cast<float>(0x7f80'0000);
    static_assert(std::numeric_limits<float>::infinity() == PLUS_INF);
    static_assert(std::numeric_limits<float>::max() == PLUS_HUGE);
    static_assert(std::numeric_limits<float>::lowest() == MINUS_HUGE);
    static_assert(std::numeric_limits<float>::denorm_min() == PLUS_TINY);
};
template <> struct TypeTraits<double> {
    using Widen = double;
    using SameSizeInt = int64;
    static CE bool integral = false;
    static CE bool floating = true;
    static CE bool fractional = true;
    static CE bool is_signed = true;
    static CE uint64 SIGN_BIT = 0x8000'0000'0000'0000;
    static CE uint64 EXPONENT_MASK = 0x7ff0'0000'0000'0000;
    static CE double MINUS_INF = std::bit_cast<double>(0xfff0'0000'0000'0000);
    static CE double MINUS_HUGE = std::bit_cast<double>(0xffef'ffff'ffff'ffff);
    static CE double MINUS_TINY = std::bit_cast<double>(0x8000'0000'0000'0001);
    static CE double MINUS_ZERO = std::bit_cast<double>(0x8000'0000'0000'0000);
    static CE double PLUS_ZERO = std::bit_cast<double>(uint64(0x0000'0000'0000'0000));
    static CE double PLUS_TINY = std::bit_cast<double>(uint64(0x0000'0000'0000'0001));
    static CE double PLUS_HUGE = std::bit_cast<double>(0x7fef'ffff'ffff'ffff);
    static CE double PLUS_INF = std::bit_cast<double>(0x7ff0'0000'0000'0000);
    static_assert(std::numeric_limits<double>::infinity() == PLUS_INF);
    static_assert(std::numeric_limits<double>::max() == PLUS_HUGE);
    static_assert(std::numeric_limits<double>::lowest() == MINUS_HUGE);
    static_assert(std::numeric_limits<double>::denorm_min() == PLUS_TINY);
};
 // long double is not supported by this library

 // Whether this is an integral type.  It is expected that you can cast from
 // integer literals and do basic arithmetic operations.
template <class T>
concept Integral = TypeTraits<T>::integral;

template <class T>
concept SignedIntegral = Integral<T> && TypeTraits<T>::is_signed;

template <class T>
concept UnsignedIntegral = Integral<T> && !TypeTraits<T>::is_signed;

 // Get a wider version of the type for multiplication.  Does not widen floats,
 // doubles, or int64s.
template <class T>
using Widen = TypeTraits<T>::Widen;

 // TODO: get rid of this
template <class A, class B>
CE auto wide_multiply (A a, B b) {
    return Widen<A>(a) * Widen<B>(b);
}

template <class T>
using MakeUnsigned = TypeTraits<T>::MakeUnsigned;

template <class T>
using MakeSigned = TypeTraits<T>::MakeSigned;

 // Strictly floating point types (float, double) or types that can use the same
 // algorithms on them (with sign-exponent-mantissa representation).
template <class T>
concept Floating = TypeTraits<T>::floating;

 // Get an integer that's the same size as the given float.
template <Floating T>
using SameSizeInt = TypeTraits<T>::SameSizeInt;

 // Types that can store numbers inbetween 0 and 1, not necessarily floating
 // point (though currently this library doesn't provide any fraction
 // non-floating types).  It is expected that fractional numbers can be cast
 // from integers and have basic arithmetic operations as well as trunc, round,
 // etc.
template <class T>
concept Fractional = TypeTraits<T>::fractional;

 // Captures pointer-like types
template <class T>
concept Pointing = requires (T p) {
    *p; p[0]; p+1; p++; p-1; p--; p - p; p == p; p != p;
};

 // Exact equality for everything but floats
template <class T> requires (!Floating<std::decay_t<T>>)
bool exact_eq (T&& a, T&& b) {
    return std::forward<T>(a) == std::forward<T>(b);
}

} // namespace geo
