#pragma once

#include <limits>
#include "../uni/common.h"
#include "../uni/macros.h"

namespace geo {
using namespace uni;

// Unlike std::is_integral, this is false for bool and char types and also
// references to ints.
template <class T>
struct IsIntegralS { static CE bool value = false; };
template <> struct IsIntegralS<int8> { static CE bool value = true; };
template <> struct IsIntegralS<uint8> { static CE bool value = true; };
template <> struct IsIntegralS<int16> { static CE bool value = true; };
template <> struct IsIntegralS<uint16> { static CE bool value = true; };
template <> struct IsIntegralS<int32> { static CE bool value = true; };
template <> struct IsIntegralS<uint32> { static CE bool value = true; };
template <> struct IsIntegralS<int64> { static CE bool value = true; };
template <> struct IsIntegralS<uint64> { static CE bool value = true; };

template <class T>
concept Integral = IsIntegralS<T>::value;

template <class T>
concept Signed = Integral<T> && std::numeric_limits<T>::is_signed;

template <class T>
concept Unsigned = Integral<T> && std::numeric_limits<T>::is_unsigned;

///// WIDENING MULTIPLICATION
// wide_multiply(a, b) is exactly like a * b, except that if a and b are
// integeral types smaller than 64 bits, it widens them to the next widest type
// before multiplying.

template <class T>
struct WidenS { using type = T; };
template <> struct WidenS<int8> { using type = int16; };
template <> struct WidenS<uint8> { using type = uint16; };
template <> struct WidenS<int16> { using type = int32; };
template <> struct WidenS<uint16> { using type = uint32; };
template <> struct WidenS<int32> { using type = int64; };
template <> struct WidenS<uint32> { using type = uint64; };

template <class T>
using Widen = WidenS<T>::type;

template <class A, class B>
CE auto wide_multiply (A&& a, B&& b) {
     // Use implicit coercion to widen
    return std::forward<Widen<A>>(a) * std::forward<Widen<B>>(b);
}

///// PREFERRED LERPING TYPE
// Determines the type of the 't' parameter passed to lerp().  Basically it's
// double for everything except float for floats.

template <class T>
struct PreferredLerperS { using type = double; };
template <>
struct PreferredLerperS<float> { using type = float; };

template <class T>
using PreferredLerper = PreferredLerperS<T>::type;

} // namespace geo
