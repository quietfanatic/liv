// Generic 1-dimensional fixed-size vectors.

#pragma once

#include <cmath>  // for sqrt

#include "../hacc/haccable.h"
#include "scalar.h"

namespace geo {

template <class T, usize n>
struct GVec;

using Vec = GVec<float, 2>;
using DVec = GVec<double, 2>;
using IVec = GVec<int32, 2>;
using LVec = GVec<int64, 2>;
using BVec = GVec<bool, 2>;

using Vec3 = GVec<float, 3>;
using DVec3 = GVec<double, 3>;
using IVec3 = GVec<int32, 3>;
using LVec3 = GVec<int64, 3>;
using BVec3 = GVec<bool, 3>;

using Vec4 = GVec<float, 4>;
using DVec4 = GVec<double, 4>;
using IVec4 = GVec<int32, 4>;
using LVec4 = GVec<int64, 4>;
using BVec4 = GVec<bool, 4>;

template <class T, usize n>
struct GVecStorage {
    T e [n];
};
template <class T>
struct GVecStorage<T, 2> {
    union {
        T e [2];
        struct {
            T x;
            T y;
        };
    };
};
template <class T>
struct GVecStorage<T, 3> {
    union {
        T e [3];
        struct {
            T x;
            T y;
            T z;
        };
    };
};
template <class T>
struct GVecStorage<T, 4> {
    union {
        T e [4];
        struct {
            T x;
            T y;
            T z;
            T w;
        };
    };
};

 // TODO: std::get and tuple_size
template <class T, usize n>
struct GVec : GVecStorage<T, n> {
    CE GVec () : GVecStorage<T, n>{.e = {}} { }
    template <class... Args, std::enable_if_t<sizeof...(Args) == n, bool> = true>
    CE GVec (Args... args) : GVecStorage<T, n>{.e = {T(args)...}} { }
    explicit CE GVec (T v) {
        for (usize i = 0; i < n; i++) {
            this->e[i] = v;
        }
    }
    template <class = std::void_t<decltype(T(NAN))>>
    CE GVec (NAN_t nan) : GVec(T(nan)) { }
    template <class = std::void_t<decltype(T(INF))>>
    CE GVec (INF_t i) : GVec(T(i)) { }
    template <class = std::void_t<decltype(T(MIN))>>
    CE GVec (MINMAX_t m) : GVec(T(m)) { }
    template <class T2>
    CE GVec (const GVec<T2, n>& o) {
        for (usize i = 0; i < n; i++) {
            this->e[i] = o[i];
        }
    }

    CE T& operator [] (usize i) {
        DA(i < n);
        return this->e[i];
    }
    CE const T& operator [] (usize i) const {
        DA(i < n);
        return this->e[i];
    }
    explicit CE operator bool () const {
        for (usize i = 0; i < n; i++) {
            if (!this->e[i]) return false;
        }
        return true;
    }
};

///// COMPARISONS

#define GVEC_COMPARISON_OP(op, res, def) \
template <class TA, class TB, usize n> \
CE bool operator op (const GVec<TA, n>& a, const GVec<TB, n>& b) { \
    for (usize i = 0; i < n; i++) { \
        if (a[i] != b[i]) return res; \
    } \
    return def; \
}
GVEC_COMPARISON_OP(==, false, true)
GVEC_COMPARISON_OP(!=, true, false)
GVEC_COMPARISON_OP(<, a[i] < b[i], false)
GVEC_COMPARISON_OP(<=, a[i] <= b[i], true)
GVEC_COMPARISON_OP(>, a[i] > b[i], false)
GVEC_COMPARISON_OP(>=, a[i] >= b[i], true)
#undef GVEC_COMPARISON_OP

///// UNARY

#define GVEC_UNARY_OP(op) \
template <class T, usize n> \
CE auto operator op (const GVec<T, n>& a) { \
    GVec<decltype(op a[0]), n> r; \
    for (usize i = 0; i < n; i++) { \
        r[i] = op a[i]; \
    } \
    return r; \
}
GVEC_UNARY_OP(+)
GVEC_UNARY_OP(-)
 // Do not define ! because the user probably expects it to coerce to scalar.
 // You can use ~ on BVecs instead
GVEC_UNARY_OP(~)

///// BINARY

#define GVEC_BINARY_OP(op) \
template <class TA, class TB, usize n> \
CE auto operator op (const GVec<TA, n>& a, const GVec<TB, n>& b) { \
    GVec<decltype(a[0] op b[0]), n> r; \
    for (usize i = 0; i < n; i++) { \
        r[i] = a[i] op b[i]; \
    } \
    return r; \
} \
template <class TA, class TB, usize n> \
CE auto operator op (const GVec<TA, n>& a, TB b) { \
    GVec<decltype(a[0] op b), n> r; \
    for (usize i = 0; i < n; i++) { \
        r[i] = a[i] op b; \
    } \
    return r; \
} \
template <class TA, class TB, usize n> \
CE auto operator op (TA a, const GVec<TB, n>& b) { \
    GVec<decltype(a op b[0]), n> r; \
    for (usize i = 0; i < n; i++) { \
        r[i] = a op b[i]; \
    } \
    return r; \
}
GVEC_BINARY_OP(+)
GVEC_BINARY_OP(-)
GVEC_BINARY_OP(*)
GVEC_BINARY_OP(/)
GVEC_BINARY_OP(%)
GVEC_BINARY_OP(|)  // These can be used on BVec
GVEC_BINARY_OP(&)
GVEC_BINARY_OP(^)
GVEC_BINARY_OP(<<)
GVEC_BINARY_OP(>>)
#undef GVEC_BINARY_OP

#define GVEC_ASSIGN_OP(op) \
template <class TA, class TB, usize n> \
CE GVec<TA, n>& operator op (GVec<TA, n>& a, const GVec<TB, n>& b) { \
    for (usize i = 0; i < n; i++) { \
        a[i] op b[i]; \
    } \
    return a; \
} \
template <class TA, class TB, usize n> \
CE GVec<TA, n>& operator op (GVec<TA, n>& a, TB b) { \
    for (usize i = 0; i < n; i++) { \
        a[i] op b; \
    } \
    return a; \
}
GVEC_ASSIGN_OP(+=)
GVEC_ASSIGN_OP(-=)
GVEC_ASSIGN_OP(*=)
GVEC_ASSIGN_OP(/=)
GVEC_ASSIGN_OP(|=)
GVEC_ASSIGN_OP(&=)
GVEC_ASSIGN_OP(^=)
GVEC_ASSIGN_OP(<<=)
GVEC_ASSIGN_OP(>>=)
#undef GVEC_ASSIGN_OP

///// SCALAR-LIKE FUNCTIONS

 // Will debug assert if some but not all elements are defined
template <class T, usize n>
CE bool defined (const GVec<T, n>& a) {
    DA([&]{
        bool any_defined = false;
        bool all_defined = true;
        for (usize i = 0; i < n; i++) {
            if (defined(a[i])) any_defined = true;
            else all_defined = false;
        }
        return any_defined == all_defined;
    }());
    return defined(a[0]);
}

template <class T, usize n>
CE bool finite (const GVec<T, n>& a) {
    for (usize i = 0; i < n; i++) {
        if (!finite(a[i])) return false;
    }
    return true;
}

template <class T, usize n>
CE auto round (const GVec<T, n>& a) {
    GVec<decltype(round(a[0])), n> r;
    for (usize i = 0; i < n; i++) {
        r[i] = round(a[i]);
    }
    return r;
}
template <class T, usize n>
CE auto floor (const GVec<T, n>& a) {
    GVec<decltype(floor(a[0])), n> r;
    for (usize i = 0; i < n; i++) {
        r[i] = floor(a[i]);
    }
    return r;
}
template <class T, usize n>
CE auto ceil (const GVec<T, n>& a) {
    GVec<decltype(ceil(a[0])), n> r;
    for (usize i = 0; i < n; i++) {
        r[i] = ceil(a[i]);
    }
    return r;
}

template <class TA, class TB, usize n>
CE auto mod (const GVec<TA, n>& a, const GVec<TB, n>& b) {
    GVec<decltype(mod(a[0], b[0])), n> r;
    for (usize i = 0; i < n; i++) {
        r[i] = mod(a[i], b[i]);
    }
    return r;
}
template <class TA, class TB, usize n>
CE auto rem (const GVec<TA, n>& a, const GVec<TB, n>& b) {
    GVec<decltype(rem(a[0], b[0])), n> r;
    for (usize i = 0; i < n; i++) {
        r[i] = rem(a[i], b[i]);
    }
    return r;
}

///// VECTOR FUNCTIONS

template <class T, usize n>
CE T length2 (const GVec<T, n>& a) {
    T r = 0;
    for (usize i = 0; i < n; i++) {
        r += a[i] * a[i];
    }
    return r;
}
 // May be double or float
template <class T, usize n>
CE auto length (const GVec<T, n>& a) {
    return std::sqrt(length2(a));
}

template <class TA, class TB>
CE auto distance2 (const TA& a, const TB& b) {
    return length2(b - a);
}

template <class TA, class TB>
CE auto distance (const TA& a, const TB& b) {
    return length(b - a);
}

 // Can be negative.
 // For 2-Vecs, equivalent to area(GRect{{0}, a})
template <class T, usize n>
CE T area (const GVec<T, n>& a) {
    T r = 1;
    for (usize i = 0; i < n; i++) {
        r *= a[i];
    }
    return r;
}

template <class T, usize n>
CE GVec<T, n> normalize (const GVec<T, n>& a) {
    return a ? a / length(a) : a;
}

template <class T, usize n>
CE T dot (const GVec<T, n>& a, const GVec<T, n>& b) {
    T r = 0;
    for (usize i = 0; i < n; i++) {
        r += a[i] * b[i];
    }
    return r;
}

template <class T, usize n>
CE GVec<T, n> lerp (
    const GVec<T, n>& a,
    const GVec<T, n>& b,
    std::conditional_t<std::is_same_v<T, float>, float, double> t
) {
    GVec<T, n> r;
    for (usize i = 0; i < n; i++) {
        r[i] = lerp(a[i], b[i], t);
    }
    return r;
}

///// FUNCTIONS FOR SPECIFIC DIMENSIONS

template <class T>
CE T slope (const GVec<T, 2>& a) {
    return a.y / a.x;
}

template <class T>
CE GVec<T, 3> cross (const GVec<T, 3>& a, const GVec<T, 3>& b) {
    return GVec<T, 3>{
       a.y * b.z - a.z * b.y,
       a.z * b.x - a.x * b.z,
       a.x * b.y - a.y * b.x
    };
}

///// GENERIC FUNCTIONS

template <class F, class T, usize n>
CE auto map (const F& f, const GVec<T, n>& a) {
    GVec<decltype(f(a[0])), n> r;
    for (usize i = 0; i < n; i++) {
        r[i] = f(a[i]);
    }
    return r;
}

} // namespace geo

///// GENERIC HACCABILITY

HACCABLE_TEMPLATE(
    HACCABLE_TEMPLATE_PARAMS(class T, usize n),
    HACCABLE_TEMPLATE_TYPE(geo::GVec<T, n>),
    hcb::name([]{
        using namespace std::literals;
        if CE (std::is_same_v<T, float>) {
            if CE (n == 2) { return "geo::Vec"sv; }
            else if CE (n == 3) { return "geo::Vec3"sv; }
            else if CE (n == 4) { return "geo::Vec4"sv; }
        }
        else if CE (std::is_same_v<T, double>) {
            if CE (n == 2) { return "geo::DVec"sv; }
            else if CE (n == 3) { return "geo::DVec3"sv; }
            else if CE (n == 4) { return "geo::DVec4"sv; }
        }
        else if CE (std::is_same_v<T, int32>) {
            if CE (n == 2) { return "geo::IVec"sv; }
            else if CE (n == 3) { return "geo::IVec3"sv; }
            else if CE (n == 4) { return "geo::IVec4"sv; }
        }
        else if CE (std::is_same_v<T, int64>) {
            if CE (n == 2) { return "geo::LVec"sv; }
            else if CE (n == 3) { return "geo::LVec3"sv; }
            else if CE (n == 4) { return "geo::LVec4"sv; }
        }
        else if CE (std::is_same_v<T, bool>) {
            if CE (n == 2) { return "geo::BVec"sv; }
            else if CE (n == 3) { return "geo::BVec3"sv; }
            else if CE (n == 4) { return "geo::BVec4"sv; }
        }
        static String r = "geo::GVec<" + String(hacc::Type::CppType<T>().name())
                        + ", " + std::to_string(n) + ">";
        return Str(r);
    }),
    hcb::length(hcb::template constant<usize>(n)),
    hcb::elem_func([](geo::GVec<T, n>& v, size_t i){
        if (i < n) return hacc::Reference(&v[i]);
        else return hacc::Reference();
    })
)

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"
namespace tap {
    template <class T, usize n>
    struct Show<geo::GVec<T, n>> {
        std::string show (const geo::GVec<T, n>& v) {
            std::string r = "[" + std::to_string(v[0]);
            for (usize i = 1; i < n; i++) {
                r += ", " + std::to_string(v[i]);
            }
            return r + "]";
        }
    };
}
#endif
