// 2d rectangles stored in (left, bottom, right, top) order.

#pragma once

#include "../ayu/describe.h"
#include "common.h"
#include "range.h"
#include "vec.h"

namespace geo {
using namespace uni;

template <class T>
struct GRect;

using Rect = GRect<float>;
using DRect = GRect<double>;
using IRect = GRect<int32>;
using LRect = GRect<int64>;
 // I can imagine use cases for this.
using BRect = GRect<bool>;

 // Like ranges, Rects are considered to include the left and bottom, and
 // exclude the right and top.
template <class T>
struct GRect {
    T l;
    T b;
    T r;
    T t;

    CE GRect () : l(0), b(0), r(0), t(0) { }
    CE GRect (T l, T b, T r, T t) : l(l), b(b), r(r), t(t) {
     // You are not allowed to create a rectangle with some sides defined but
     // not others.
#ifndef NDEBUG
        bool any_defined = l == l || b == b || r == r || t == t;
        bool all_defined = l == l && b == b && r == r && t == t;
        expect(any_defined == all_defined);
#endif
    }
     // Create the undefined rectangle.  Most operations are not defined on the
     // undefined rectangle.
    CE GRect (GNAN_t n) : l(n), b(n), r(n), t(n) { }
     // Create an infinitely large (possibly negative) rectangle.
    CE GRect (GINF_t i) : l(-i), b(-i), r(i), t(i) { }
     // Create from lower-left and upper-right corners
    CE GRect (const GVec<T, 2>& lb, const GVec<T, 2>& rt) :
        l(lb.x), b(lb.y), r(rt.x), t(rt.y)
    { }
     // Create from two 1-dimensional ranges.
    CE GRect (const GRange<T>& lr, const GRange<T>& bt) :
        l(lr.l), b(bt.l), r(lr.r), t(bt.r)
    { }

     // Don't use this to check for definedness or area = 0.  It only checks
     // that each side is strictly zero.
    CE explicit operator bool () const { return l || b || r || t; }
};

template <class T>
struct TypeTraits<GRect<T>> {
    using Widened = GRect<Widen<T>>;
    static CE bool integral = false;
    static CE bool floating = false;
    static CE bool fractional = false;
    static CE bool is_signed = TypeTraits<T>::is_signed;
};

///// PROPERTIES

 // Get a corner of the rectangle
template <class T>
CE GVec<T, 2> lb (const GRect<T>& a) { return {a.l, a.b}; }
template <class T>
CE GVec<T, 2> rb (const GRect<T>& a) { return {a.r, a.b}; }
template <class T>
CE GVec<T, 2> rt (const GRect<T>& a) { return {a.r, a.t}; }
template <class T>
CE GVec<T, 2> lt (const GRect<T>& a) { return {a.l, a.t}; }

 // Get center point
template <class T>
CE GVec<T, 2> center (const GRect<T>& a) {
    return {center(lr(a)), center(bt(a))};
}

 // Get one dimension of the rectangle
template <class T>
CE GRange<T> lr (const GRect<T>& a) { return {a.l, a.r}; }
template <class T>
CE GRange<T> bt (const GRect<T>& a) { return {a.b, a.t}; }

 // Two-dimensional size
template <class T>
CE GVec<T, 2> size (const GRect<T>& a) { return {a.r - a.l, a.t - a.b}; }
 // width(a) == size(a).x == size(lr(a))
template <class T>
CE T width (const GRect<T>& a) { return a.r - a.l; }
 // height(a) == size(a).y == size(bt(a))
template <class T>
CE T height (const GRect<T>& a) { return a.t - a.b; }

 // Will debug assert if some but not all elements are undefined
template <class T>
CE bool defined (const GRect<T>& a) {
#ifndef NDEBUG
    bool any_defined = a.l == a.l || a.b == a.b || a.r == a.r || a.t == a.t;
    bool all_defined = a.l == a.l && a.b == a.b && a.r == a.r && a.t == a.t;
    expect(any_defined == all_defined);
#endif
    return defined(a.l);
}

 // Returns false if any side is NAN, INF, or -INF
template <class T>
CE bool finite (const GRect<T>& a) {
    return finite(a.l) && finite(a.b) && finite(a.r) && finite(a.t);
}

 // Will be negative if one of width() or height() is negative (but not both)
template <class T>
CE auto area (const GRect<T>& a) {
    return widen(a.r - a.l) * widen(a.t - a.b);
}

 // Area is 0 (either width or height is 0)
template <class T>
CE bool empty (const GRect<T>& a) {
    return a.l == a.r || a.b == a.t;
}

 // Both width and height are non-negative.  (proper(NAN) == true)
template <class T>
CE bool proper (const GRect<T>& a) {
    return proper(lr(a)) && proper(bt(a));
}

 // The bounding box of a rectangle is itself
template <class T>
CE const GRect<T>& bounds (const GRect<T>& a) { return a; }

///// MODIFIERS

 // Change inclusivity of sides
template <class T>
CE GRect<T> exclude_lb (const GRect<T>& a) {
    return {exclude_l(lr(a)), exclude_l(bt(a))};
}
template <class T>
CE GRect<T> include_rt (const GRect<T>& a) {
    return {include_r(lr(a)), include_r(bt(a))};
}

 // Flip both horizontally and vertically but keep the center in the same place.
 // To flip around the origin, multiply by -1 instead.
template <class T>
CE GRect<T> invert (const GRect<T>& a) {
    return {a.r, a.t, a.l, a.b};
}

 // Flip horizontally
template <class T>
CE GRect<T> invert_h (const GRect<T>& a) {
    return {a.r, a.b, a.l, a.t};
}

 // Flip vertically
template <class T>
CE GRect<T> invert_v (const GRect<T>& a) {
    return {a.l, a.t, a.r, a.b};
}

 // If not proper, flip horizontally and/or vertically to make it proper.
template <class T>
CE GRect<T> properize (const GRect<T>& a) {
    return {properize(a.lr()), properize(a.bt())};
}

 // Arithmetic operators
#define GRECT_UNARY_OP(op) \
template <class T> \
CE GRect<T> operator op (const GRect<T>& a) { \
    return {op a.l, op a.b, op a.t, op a.r}; \
}
GRECT_UNARY_OP(+)
GRECT_UNARY_OP(-)
#undef GRECT_UNARY_OP

///// RELATIONSHIPS

template <class T>
CE bool operator == (const GRect<T>& a, const GRect<T>& b) {
    return a.l == b.l && a.b == b.b && a.r == b.r && a.t == b.t;
}
template <class T>
CE bool operator != (const GRect<T>& a, const GRect<T>& b) {
    return a.l != b.l || a.b != b.b || a.r != b.r || a.t != b.t;
}

// These assume the rectangles are proper, and may give unintuitive results
// if they aren't.

 // a and b are overlapping.  Returns false if the rectangles are only touching
 // on the border.
 // overlaps(a, b) == !empty(a & b)
template <class T>
CE bool overlaps (const GRect<T>& a, const GRect<T>& b) {
    return overlaps(lr(a), lr(b)) && overlaps(bt(a), bt(b));
}
 // touches(a, b) == proper(a & b)
template <class T>
CE bool touches (const GRect<T>& a, const GRect<T>& b) {
    return touches(lr(a), lr(b)) && touches(bt(a), bt(b));
}

 // b is fully contained in a.
 // contains(a, b) == ((a | b) == a) == ((a & b) == b)
template <class T>
CE bool contains (const GRect<T>& a, const GRect<T>& b) {
    return contains(lr(a), lr(b)) && contains(bt(a), bt(b));
}
 // b is contained in a.  Note that the left and bottom are inclusive but the
 // right and top are exclusive.
template <class T>
CE bool contains (const GRect<T>& a, const GVec<T, 2>& b) {
    return contains(lr(a), b.x) && contains(bt(a), b.y);
}

///// COMBINERS

#define GRECT_GVEC_OP(op) \
template <class T> \
CE GRect<T> operator op (const GRect<T>& a, const GVec<T, 2>& b) { \
    return {a.l op b.x, a.b op b.y, a.r op b.x, a.t op b.y}; \
} \
template <class T> \
CE GRect<T> operator op (const GVec<T, 2>& a, const GRect<T>& b) { \
    return {a.x op b.l, a.y op b.b, a.x op b.r, a.y op b.t}; \
}
GRECT_GVEC_OP(+)
GRECT_GVEC_OP(-)
GRECT_GVEC_OP(*)
GRECT_GVEC_OP(/)
#undef GRECT_GVEC_OP

#define GRECT_GVEC_OPEQ(op) \
template <class T> \
CE GRect<T>& operator op (GRect<T>& a, const GVec<T, 2>& b) { \
    a.l op b.x; a.b op b.y; a.r op b.x; a.t op b.y; \
    return a; \
}
GRECT_GVEC_OPEQ(+=)
GRECT_GVEC_OPEQ(-=)
GRECT_GVEC_OPEQ(*=)
GRECT_GVEC_OPEQ(/=)
#undef GRECT_GVEC_OPEQ

#define GRECT_SCALAR_OP(op) \
template <class T> \
CE GRect<T> operator op (const GRect<T>& a, T b) { \
    return {a.l op b, a.b op b, a.r op b, a.t op b}; \
} \
template <class T> \
CE GRect<T> operator op (T a, const GRect<T>& b) { \
    return {a op b.l, a op b.b, a op b.r, a op b.t}; \
}
GRECT_SCALAR_OP(*)
GRECT_SCALAR_OP(/)
#undef GRECT_SCALAR_OP

#define GRECT_SCALAR_OPEQ(op) \
template <class T> \
CE GRect<T>& operator op (GRect<T>& a, T b) { \
    a.l op b; a.b op b; a.r op b; a.t op b; \
    return a; \
}
GRECT_SCALAR_OPEQ(*=)
GRECT_SCALAR_OPEQ(/=)
#undef GRECT_SCALAR_OPEQ

 // Box union
template <class T>
CE GRect<T> operator | (const GRect<T>& a, const GRect<T>& b) {
    return {min(a.l, b.l), min(a.b, b.b), max(a.r, b.r), max(a.t, b.t)};
}
template <class T>
CE GRect<T>& operator |= (GRect<T>& a, const GRect<T>& b) {
    return a = a | b;
}
 // Box intersection.  If a and b aren't intersecting, the result is not proper.
template <class T>
CE GRect<T> operator & (const GRect<T>& a, const GRect<T>& b) {
    return {max(a.l, b.l), max(a.b, b.b), min(a.r, b.r), min(a.t, b.t)};
}
template <class T>
CE GRect<T>& operator &= (GRect<T>& a, const GRect<T>& b) {
    return a = a & b;
}

template <class A, class B, Fractional T>
CE auto lerp (
    const GRect<A>& a,
    const GRect<B>& b,
    T t
) {
    return GRect<decltype(lerp(a.l, b.l, t))>{
        lerp(a.l, b.l, t),
        lerp(a.b, b.b, t),
        lerp(a.r, b.r, t),
        lerp(a.t, b.t, t)
    };
}

 // If p is outside of a, returns the closest point to p contained in a.
template <class T>
CE GVec<T, 2> clamp (const GVec<T, 2>& p, const GRect<T>& a) {
    return {clamp(p.x, a.lr()), clamp(p.y, a.bt())};
}

} // namespace geo

///// GENERIC AYU DESCRIPTION

AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(geo::GRect<T>),
    desc::name([]{
        using namespace std::literals;
        using namespace uni;
        if CE (std::is_same_v<T, float>) return "geo::Rect"sv;
        else if CE (std::is_same_v<T, double>) return "geo::DRect"sv;
        else if CE (std::is_same_v<T, int32>) return "geo::IRect"sv;
        else if CE (std::is_same_v<T, int64>) return "geo::LRect"sv;
        else if CE (std::is_same_v<T, bool>) return "geo::BRect"sv;
        else {
            static String r = "geo::GRect<" + String(
                ayu::Type::CppType<T>().name()
            ) + ">";
            return Str(r);
        }
    }),
    desc::elems(
        desc::elem(&geo::GRect<T>::l),
        desc::elem(&geo::GRect<T>::b),
        desc::elem(&geo::GRect<T>::r),
        desc::elem(&geo::GRect<T>::t)
    )
)
