#pragma once

#include "../uni/common.h"
#include "../uni/macros.h"

namespace geo {
using namespace uni;

///// WIDENING MULTIPLICATION
// wide_multiply(a, b) is exactly like a * b, except that if a and b are
// integeral types smaller than 64 bits, it widens them to the next widest type
// before multiplying.

template <class T>
struct WidenS { using type = T; };
template <>
struct WidenS<int8> { using type = int16; };
template <>
struct WidenS<uint8> { using type = uint16; };
template <>
struct WidenS<int16> { using type = int32; };
template <>
struct WidenS<uint16> { using type = uint32; };
template <>
struct WidenS<int32> { using type = int64; };
template <>
struct WidenS<uint32> { using type = uint64; };

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
