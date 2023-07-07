#pragma once

#include "../ayu/describe.h"
#include "common.h"
#include "scalar.h"

namespace geo {

 // Ranges.  Inclusive on the left side and exclusive on the right side.
template <class T>
struct GRange;

using Range = GRange<float>;
using DRange = GRange<double>;
using IRange = GRange<int32>;
using LRange = GRange<int64>;
using BRange = GRange<bool>;

template <class T>
struct GRange {
    T l;
    T r;
    constexpr GRange () : l(), r() { }
    constexpr GRange (GNAN_t n) : l(n), r(n) { }
    constexpr GRange (GINF_t i) : l(-i), r(i) { }
    template <class A, class B>
    constexpr GRange (A l, B r) : l(l), r(r) {
        expect(valid(*this));
    }
     // Why am I even doing this, this is so dumb
    template <class Ix>
    constexpr auto operator [] (Ix i) const {
        expect(i < r - l);
        return l[i];
    }
     // No operator bool because it's not clear whether it should check if the
     // range is empty (size == 0) or if the range is {0, 0}
};

template <class T>
struct TypeTraits<GRange<T>> {
    using Widened = GRange<Widen<T>>;
    static constexpr bool integral = false;
    static constexpr bool floating = false;
    static constexpr bool fractional = false;
    static constexpr bool is_signed = TypeTraits<T>::is_signed;
};

///// PROPERTIES

 // If T is a pointer, supports range-for-loops.  Yay!
template <class T>
constexpr const T& begin (const GRange<T>& a) { return a.l; }
template <class T>
constexpr const T& end (const GRange<T>& a) { return a.r; }
template <class T>
constexpr auto size (const GRange<T>& a) { return a.r - a.l; }

template <class T>
constexpr auto center (const GRange<T>& a) { return (a.l + a.r) / 2; }

template <class T>
constexpr bool valid (const GRange<T>& a) {
    if constexpr (requires (T v) { defined(v); }) {
        return defined(a.l) == defined(a.r);
    }
    else return true;
}

template <class T>
constexpr bool defined (const GRange<T>& a) {
    expect(valid(a));
    return defined(a.l);
}

template <class T>
constexpr bool finite (const GRange<T>& a) {
    return finite(a.l) && finite(a.r);
}

template <class T>
constexpr bool empty (const GRange<T>& a) {
    return a.l == a.r;
}

template <class T>
constexpr bool proper (const GRange<T>& a) {
    return a.l <= a.r;
}

///// MODIFIERS

 // Change inclusivity
template <class T>
constexpr GRange<T> exclude_l (const GRange<T>& a) {
    return {next_quantum(a.l), a.r};
}
template <class T>
constexpr GRange<T> include_r (const GRange<T>& a) {
    return {a.l, next_quantum(a.r)};
}

template <class T>
constexpr GRange<T> invert (const GRange<T>& a) {
    return {a.r, a.l};
}

template <class T>
constexpr GRange<T> properize (const GRange<T>& a) {
    return proper(a) ? a : invert(a);
}

///// RELATIONSHIPS

template <class T>
constexpr bool operator== (const GRange<T>& a, const GRange<T>& b) {
    return a.l == b.l && a.r == b.r;
}

#define GRANGE_UNARY_OP(op) \
template <class T> \
constexpr auto operator op (const GRange<T>& a) { \
    return GRange<decltype(op a.l)>{op a.l, op a.r}; \
}
GRANGE_UNARY_OP(+)
GRANGE_UNARY_OP(-)
GRANGE_UNARY_OP(~)
#undef GRANGE_UNARY_OP

 // These assume the ranges are proper, and may give unintuitive results if they
 // aren't.

 // Returns true if the ranges are strictly overlapping (not just touching).
 // overlaps(a, b) == !empty(a & b)
template <class T>
constexpr bool overlaps (const GRange<T>& a, const GRange<T>& b) {
    return a.l < b.r && a.r < b.l;
}
 // Returns true if overlapping or touching.
 // touches(a, b) == proper(a & b)
template <class T>
constexpr bool touches (const GRange<T>& a, const GRange<T>& b) {
    return a.l <= b.r && a.r <= b.l;
}

 // b is fully contained in a.
 // contains(a, b) == ((a | b) == a) == ((a & b) == b)
template <class T>
constexpr bool contains (const GRange<T>& a, const GRange<T>& b) {
    return a.l <= b.l && b.r <= a.r;
}
 // contains(a, b) == (clamp(b, a) == b)
template <class T>
constexpr bool contains (const GRange<T>& a, const T& b) {
    return a.l <= b && b < a.r;
}

///// COMBINERS

#define GRANGE_BINARY_OP(op) \
template <class TA, class TB> \
constexpr auto operator op (const GRange<TA>& a, const TB& b) { \
    return GRange<decltype(a.l op b)>{a.l op b, a.r op b}; \
} \
template <class TA, class TB> \
constexpr auto operator op (const TA& a, const GRange<TB>& b) { \
    return GRange<decltype(a op b.l)>{a op b.l, a op b.r}; \
}
GRANGE_BINARY_OP(+)
GRANGE_BINARY_OP(-)
GRANGE_BINARY_OP(*)
GRANGE_BINARY_OP(/)
 // Other operators don't really make sense for ranges.  Also, we define our own
 // | and & later on.
#undef GRANGE_BINARY_OP

#define GRANGE_ASSIGN_OP(op) \
template <class TA, class TB> \
constexpr GRange<TA> operator op (GRange<TA>& a, const TB& b) { \
    a.l op b; \
    a.r op b; \
    return a; \
}
GRANGE_ASSIGN_OP(+=)
GRANGE_ASSIGN_OP(-=)
GRANGE_ASSIGN_OP(*=)
GRANGE_ASSIGN_OP(/=)
#undef GRANGE_ASSIGN_OP

 // Range union, like for Rects but 1-dimensional
template <class T>
constexpr GRange<T> operator | (const GRange<T>& a, const GRange<T>& b) {
    return {min(a.l, b.l), max(a.r, b.r)};
}
template <class T>
constexpr GRange<T>& operator |= (GRange<T>& a, const GRange<T>& b) {
    return a = a | b;
}
 // Range intersection.  If a and b aren't intersecting, the result is not
 // proper.
template <class T>
constexpr GRange<T> operator & (const GRange<T>& a, const GRange<T>& b) {
    return {max(a.l, b.l), min(a.r, b.r)};
}
template <class T>
constexpr GRange<T>& operator &= (GRange<T>& a, const GRange<T>& b) {
    return a = a & b;
}

 // If p is outside of a, returns the closest value to p contained in a.  Note
 // that because the right side of the range is exclusive, this will never
 // return a.r.  To allow returning a.r, use clamp(p, r.include_r()).
template <class TA, class TB>
constexpr TA clamp (const TA& p, const GRange<TB>& r) {
    return p < r.l ? TA(r.l) : p >= r.r ? TB(prev_quantum(r.r)) : p;
}

 // Lerp between two ranges
template <class A, class B, Fractional T>
constexpr auto lerp (
    const GRange<A>& a, const GRange<B>& b, const T& t
) {
    return GRange<decltype(lerp(a.l, b.l, t))>(
        lerp(a.l, b.l, t), lerp(a.r, b.r, t)
    );
}

///// MISC

 // Lerp within one range
template <class A, Fractional T>
constexpr A lerp (const GRange<A>& a, T t) {
    return lerp(a.l, a.r, t);
}

} // namespace geo

///// GENERIC AYU DESCRIPTION

AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(geo::GRange<T>),
    desc::name([]()->uni::StaticString{
        if constexpr (std::is_same_v<T, float>) return "geo::Range";
        else if constexpr (std::is_same_v<T, double>) return "geo::DRange";
        else if constexpr (std::is_same_v<T, uni::int32>) return "geo::IRange";
        else if constexpr (std::is_same_v<T, uni::int64>) return "geo::LRange";
        else if constexpr (std::is_same_v<T, bool>) return "geo::BRange";
        else {
            static uni::UniqueString r = uni::cat(
                "geo::GRange<", ayu::Type::CppType<T>().name(), '>'
            );
            return uni::StaticString::Static(r);
        }
    }),
    desc::elems(
        desc::elem(&geo::GRange<T>::l),
        desc::elem(&geo::GRange<T>::r)
    )
)
