// 2d rectangles stored in (left, bottom, right, top) order.

#pragma once

#include "vec.h"

namespace geo {

template <class T>
struct GRect;

using Rect = GRect<float>;
using DRect = GRect<double>;
using IRect = GRect<int32>;
using LRect = GRect<int64>;
 // I can imagine use cases for this.
using BRect = GRect<bool>;

template <class T>
struct GRect {
    T l;
    T b;
    T r;
    T t;

    CE GRect () : l(0), b(0), r(0), t(0) { }
    CE GRect (NAN_t n) : l(n), b(n), r(n), t(n) { }
    CE GRect (INF_t i) : l(-i), b(-i), r(i), t(i) { }
    CE GRect (MINMAX_t m) : l(-m), b(-m), r(m), t(m) { }
    CE GRect (T l, T b, T r, T t) : l(l), b(b), r(r), t(t) { }
    CE GRect (const GVec<T, 2>& lb, const GVec<T, 2>& rt) :
        l(lb.x), b(lb.y), r(rt.x), t(rt.y)
    { }

    CE GVec<T, 2> lb () const { return {l, b}; }
    CE GVec<T, 2> rb () const { return {r, b}; }
    CE GVec<T, 2> rt () const { return {r, t}; }
    CE GVec<T, 2> lt () const { return {l, t}; }

    CE T width () const { return r - l; }
    CE T height () const { return t - b; }
    CE GVec<T, 2> size () const { return {width(), height()}; }

    CE operator bool () const { return l || b || r || t; }
};

///// OPERATORS

template <class T>
CE bool operator == (const GRect<T>& a, const GRect<T>& b) {
    return a.l == b.l && a.b == b.b && a.r == b.r && a.t == b.t;
}
template <class T>
CE bool operator != (const GRect<T>& a, const GRect<T>& b) {
    return a.l != b.l || a.b != b.b || a.r != b.r || a.t != b.t;
}

template <class T>
CE GRect<T> operator - (const GRect<T>& a) {
    return {-a.l, -a.b, -a.t, -a.r};
}

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

///// RECTANGLE FUNCTIONS

 // Will debug assert if some but not all elements are undefined
template <class T>
CE bool defined (const GRect<T>& a) {
#ifdef NDEBUG
    return defined(a.l);
#else
    bool any_defined = a.l == a.l || a.b == a.b || a.r == a.r || a.t == a.t;
    bool all_defined = a.l == a.l && a.b == a.b && a.r == a.r && a.t == a.t;
    AA(any_defined == all_defined);
    return all_defined;
#endif
}

template <class T>
CE bool finite (const GRect<T>& a) {
    return finite(a.l) && finite(a.b) && finite(a.r) && finite(a.t);
}

 // Will be negative if one of width() or height() is negative (but not both)
template <class T>
CE T area (const GRect<T>& a) {
    return (a.r - a.l) * (a.t - a.b);
}

 // Area is 0 (either width or height is 0)
template <class T>
CE bool empty (const GRect<T>& a) {
    return area(a) == 0;
}

 // The bounding box of a rectangle is itself
template <class T>
CE const GRect<T>& bounds (const GRect<T>& a) { return a; }

 // Flip both horizontally and vertically but keep the center in the same place.
template <class T>
CE GRect<T> invert (const GRect<T>& a) {
    return {a.r, a.t, a.l, a.b};
}

template <class T>
CE GRect<T> invert_h (const GRect<T>& a) {
    return {a.r, a.b, a.l, a.t};
}

template <class T>
CE GRect<T> invert_v (const GRect<T>& a) {
    return {a.l, a.t, a.r, a.b};
}

 // Both width and height are non-negative.  (proper(NAN) == true)
template <class T>
CE bool proper (const GRect<T>& a) {
    return a.r >= a.l && a.t >= a.b;
}

 // If not proper, flip horizontally and/or vertically to make it proper.
template <class T>
CE GRect<T> properize (const GRect<T>& a) {
    return {
        min(a.l, a.r),
        min(a.b, a.t),
        max(a.r, a.l),
        max(a.t, a.b)
    };
}

template <class T>
CE GRect<T> lerp (
    const GRect<T>& a,
    const GRect<T>& b,
    std::conditional_t<std::is_same_v<T, float>, float, double> t
) {
    return {
        lerp(a.l, b.l, t),
        lerp(a.b, b.b, t),
        lerp(a.r, b.r, t),
        lerp(a.t, b.t, t)
    };
}

///// RELATIONSHIPS
// These assume the rectangles are proper, and may give unintuitive results
//  if they aren't.

 // a and b share area or are touching on the border
template <class T>
CE bool touches (const GRect<T>& a, const GRect<T>& b) {
    return a.l <= b.r && a.r <= b.l
        && a.b <= b.t && a.t <= b.b;
}
template <class T>
CE bool touches (const GRect<T>& a, const GVec<T, 2>& b) {
    return a.l <= b.x && b.x <= a.r
        && a.b <= b.y && b.y <= a.t;
}

 // a and b share area (aren't just touching on the border)
template <class T>
CE bool overlaps (const GRect<T>& a, const GRect<T>& b) {
    return a.l < b.r && a.r < b.l
        && a.b < b.t && a.t < b.b;
}
template <class T>
CE bool overlaps (const GRect<T>& a, const GVec<T, 2>& b) {
    return a.l < b.x && b.x < a.r
        && a.b < b.y && b.y < a.t;
}

 // b is fully contained in a, but they can share borders
template <class T>
CE bool contains (const GRect<T>& a, const GVec<T, 2>& b) {
    return a.l <= b.x && b.x <= a.r
        && a.b <= b.y && b.y <= a.t;
}
template <class T>
CE bool contains (const GRect<T>& a, const GRect<T>& b) {
    return contains(a, b.lb()) && contains(a, b.rt());
}

 // b is fully contained in a, and they can't share borders
template <class T>
CE bool covers (const GRect<T>& a, const GVec<T, 2>& b) {
    return a.l < b.x && b.x < a.r
        && a.b < b.y && b.y < a.t;
}
template <class T>
CE bool covers (const GRect<T>& a, const GRect<T>& b) {
    return covers(a, b.lb()) && covers(a, b.rt());
}

 // If p is outside of a, returns the closest point to p contained in a.
template <class T>
CE GVec<T, 2> snap (const GRect<T>& a, const GVec<T, 2>& p) {
    return {
        p.x < a.l ? a.l : p.x > a.r ? a.r : p.x,
        p.y < a.b ? a.b : p.y > a.t ? a.t : p.y
    };
}

} // namespace geo

HACCABLE_TEMPLATE(
    HACCABLE_TEMPLATE_PARAMS(class T),
    HACCABLE_TEMPLATE_TYPE(geo::GRect<T>),
    hcb::name([]{
        using namespace std::literals;
        if CE (std::is_same_v<T, float>) return "geo::Rect"sv;
        else if CE (std::is_same_v<T, double>) return "geo::DRect"sv;
        else if CE (std::is_same_v<T, int32>) return "geo::IRect"sv;
        else if CE (std::is_same_v<T, int64>) return "geo::LRect"sv;
        else if CE (std::is_same_v<T, bool>) return "geo::BRect"sv;
        else {
            static String r = "geo::GRect<" + String(hacc::Type::CppType<T>().name()) + ">";
            return Str(r);
        }
    }),
    hcb::elems(
        hcb::elem(&geo::GRect<T>::l),
        hcb::elem(&geo::GRect<T>::b),
        hcb::elem(&geo::GRect<T>::r),
        hcb::elem(&geo::GRect<T>::t)
    )
)
