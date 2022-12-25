// Generic 1-dimensional fixed-size vectors.

#pragma once

#include "../ayu/describe.h"
#include "common.h"
#include "scalar.h"

namespace geo {
using namespace uni;

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

///// VECTOR STORAGE
// Some uniony magic to allow you to reference elements with either operator[]
// or .x, .y, etc

template <class T, usize n>
using GVecStorageGeneric = T[n];

template <class T, usize n>
struct GVecStorage {
    GVecStorageGeneric<T, n> e;
};
template <class T> requires (std::is_trivially_destructible_v<T>)
struct GVecStorage<T, 2> {
    union {
        GVecStorageGeneric<T, 2> e;
        struct {
            T x;
            T y;
        };
    };
};
template <class T> requires (!std::is_trivially_destructible_v<T>)
struct GVecStorage<T, 2> {
    union {
        GVecStorageGeneric<T, 2> e;
        struct {
            T x;
            T y;
        };
    };
    ~GVecStorage () { e.~GVecStorageGeneric(); }
};
template <class T> requires (std::is_trivially_destructible_v<T>)
struct GVecStorage<T, 3> {
    union {
        GVecStorageGeneric<T, 3> e;
        struct {
            T x;
            T y;
            T z;
        };
    };
};
template <class T> requires (!std::is_trivially_destructible_v<T>)
struct GVecStorage<T, 3> {
    union {
        GVecStorageGeneric<T, 3> e;
        struct {
            T x;
            T y;
            T z;
        };
    };
    ~GVecStorage () { e.~GVecStorageGeneric(); }
};
template <class T> requires (std::is_trivially_destructible_v<T>)
struct GVecStorage<T, 4> {
    union {
        GVecStorageGeneric<T, 4> e;
        struct {
            T x;
            T y;
            T z;
            T w;
        };
    };
};
template <class T> requires (!std::is_trivially_destructible_v<T>)
struct GVecStorage<T, 4> {
    union {
        GVecStorageGeneric<T, 4> e;
        struct {
            T x;
            T y;
            T z;
            T w;
        };
    };
    ~GVecStorage () { e.~GVecStorageGeneric(); }
};

///// GVec CLASS

 // TODO: std::get and tuple_size
template <class T, usize n>
struct GVec : GVecStorage<T, n> {
     // Default constructor
    constexpr GVec () : GVecStorage<T, n>{.e = {}} { }

     // Construct from individual elements
    template <class... Args> requires(sizeof...(Args) == n)
    constexpr GVec (Args... args) :
        GVecStorage<T, n>{.e = {T(args)...}}
    {
#ifndef NDEBUG
        expect(valid(*this));
#endif
    }

     // Construct from a scalar (will be copied to all elements)
    constexpr explicit GVec (const T& v) {
        for (usize i = 0; i < n; i++) {
            this->e[i] = v;
        }
    }
     // Construct the undefined vector
    template <class = void> requires (
        requires (GNAN_t nan) { T(nan); }
    )
    constexpr GVec (GNAN_t nan) : GVec(T(nan)) { }

     // Implicitly coerce from another vector
    template <class T2>
    constexpr GVec (const GVec<T2, n>& o) {
        for (usize i = 0; i < n; i++) {
            this->e[i] = o[i];
        }
    }

     // Get individual element
    constexpr T& operator [] (usize i) {
        expect(i < n);
        return this->e[i];
    }
    constexpr const T& operator [] (usize i) const {
        expect(i < n);
        return this->e[i];
    }
     // Check for the zero vector.  Does not check definedness.
    constexpr explicit operator bool () const {
        for (usize i = 0; i < n; i++) {
            if (!this->e[i]) return false;
        }
        return true;
    }
};

template <class T, usize n>
struct TypeTraits<GVec<T, n>> {
    using Widened = GVec<Widen<T>, n>;
    static constexpr bool integral = false;
    static constexpr bool floating = false;
    static constexpr bool fractional = false;
    static constexpr bool is_signed = TypeTraits<T>::is_signed;
};

template <usize i, class T, usize n> requires (i < n)
auto get (const GVec<T, n>& a) { return a[i]; }

///// PROPERTIES

 // A Vec is valid is all elements are defined or no elements are defined.
template <class T, usize n>
constexpr bool valid (const GVec<T, n>& a) {
    if constexpr (n > 0 && requires (T v) { defined(v); }) {
        bool is_defined = defined(a[0]);
        for (usize i = 1; i < n; i++) {
            if (defined(a[0]) != is_defined) return false;
        }
    }
    return true;
}

 // Will debug assert if some but not all elements are defined
template <class T, usize n>
constexpr bool defined (const GVec<T, n>& a) {
#ifndef NDEBUG
    expect(valid(a));
#endif
    return defined(a[0]);
}

 // Returns false if any elements are NAN or INF
template <class T, usize n>
constexpr bool finite (const GVec<T, n>& a) {
    for (usize i = 0; i < n; i++) {
        if (!finite(a[i])) return false;
    }
    return true;
}

 // length squared.  Faster than length.
 // length2(a) == dot(a, a)
template <class T, usize n>
constexpr auto length2 (const GVec<T, n>& a) {
    Widen<T> r = 0;
    for (usize i = 0; i < n; i++) {
        r += widen(a[i]) * widen(a[i]);
    }
    return r;
}
template <class T, usize n>
constexpr T length (const GVec<T, n>& a) {
    return root2(length2(a));
}

 // Can be negative.
 // For 2-Vecs, equivalent to area(GRect{{0}, a})
template <class T, usize n>
constexpr auto area (const GVec<T, n>& a) {
    Widen<T> r = 1;
    for (usize i = 0; i < n; i++) {
        r *= a[i];
    }
    return r;
}

template <class T, usize n>
constexpr bool normal (const GVec<T, n>& a) {
    return length2(a) == T(1);
}

 // Slope of the line from the origin to a.
template <class T>
constexpr T slope (const GVec<T, 2>& a) {
    return a.y / a.x;
}
 // 1 / slope(a).  This is separate because floating point arithmetic doesn't
 // optimize very well.
template <class T>
constexpr T aspect (const GVec<T, 2>& a) {
    return a.x / a.y;
}

///// MODIFIERS

#define GVEC_UNARY_OP(op) \
template <class T, usize n> \
constexpr auto operator op (const GVec<T, n>& a) { \
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
#undef GVEC_UNARY_OP

 // Vector version of rounding functions that just call the same function on
 // each element.
template <class T, usize n>
constexpr auto round (const GVec<T, n>& a) {
    GVec<decltype(round(a[0])), n> r;
    for (usize i = 0; i < n; i++) {
        r[i] = round(a[i]);
    }
    return r;
}
template <class T, usize n>
constexpr auto floor (const GVec<T, n>& a) {
    GVec<decltype(floor(a[0])), n> r;
    for (usize i = 0; i < n; i++) {
        r[i] = floor(a[i]);
    }
    return r;
}
template <class T, usize n>
constexpr auto ceil (const GVec<T, n>& a) {
    GVec<decltype(ceil(a[0])), n> r;
    for (usize i = 0; i < n; i++) {
        r[i] = ceil(a[i]);
    }
    return r;
}

 // Get vector that has the same direction but with a length of 1.  Returns the
 // zero vector if given the zero vector.
template <class T, usize n>
constexpr GVec<T, n> normalize (const GVec<T, n>& a) {
    return a ? a / length(a) : a;
}

///// RELATIONSHIPS

#define GVEC_COMPARISON_OP(op, res, def) \
template <class TA, class TB, usize n> \
constexpr bool operator op (const GVec<TA, n>& a, const GVec<TB, n>& b) { \
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

///// COMBINERS

 // Binary operators with vec-vec, vec-scalar, and scalar-vec.
#define GVEC_BINARY_OP(op) \
template <class TA, class TB, usize n> \
constexpr auto operator op (const GVec<TA, n>& a, const GVec<TB, n>& b) { \
    GVec<decltype(a[0] op b[0]), n> r; \
    for (usize i = 0; i < n; i++) { \
        r[i] = a[i] op b[i]; \
    } \
    return r; \
} \
template <class TA, class TB, usize n> \
constexpr auto operator op (const GVec<TA, n>& a, const TB& b) { \
    GVec<decltype(a[0] op b), n> r; \
    for (usize i = 0; i < n; i++) { \
        r[i] = a[i] op b; \
    } \
    return r; \
} \
template <class TA, class TB, usize n> \
constexpr auto operator op (const TA& a, const GVec<TB, n>& b) { \
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
GVEC_BINARY_OP(<<)  // Not sure what you'd use these for but okay
GVEC_BINARY_OP(>>)
#undef GVEC_BINARY_OP

 // Assignment operators vec-vec and vec-scalar
#define GVEC_ASSIGN_OP(op) \
template <class TA, class TB, usize n> \
constexpr GVec<TA, n>& operator op (GVec<TA, n>& a, const GVec<TB, n>& b) { \
    for (usize i = 0; i < n; i++) { \
        a[i] op b[i]; \
    } \
    return a; \
} \
template <class TA, class TB, usize n> \
constexpr GVec<TA, n>& operator op (GVec<TA, n>& a, const TB& b) { \
    for (usize i = 0; i < n; i++) { \
        a[i] op b; \
    } \
    return a; \
}
GVEC_ASSIGN_OP(+=)
GVEC_ASSIGN_OP(-=)
GVEC_ASSIGN_OP(*=)
GVEC_ASSIGN_OP(/=)
GVEC_ASSIGN_OP(%=)
GVEC_ASSIGN_OP(|=)
GVEC_ASSIGN_OP(&=)
GVEC_ASSIGN_OP(^=)
GVEC_ASSIGN_OP(<<=)
GVEC_ASSIGN_OP(>>=)
#undef GVEC_ASSIGN_OP

 // mod and rem are basically binary operators
template <class TA, class TB, usize n>
constexpr auto mod (const GVec<TA, n>& a, const GVec<TB, n>& b) {
    GVec<decltype(mod(a[0], b[0])), n> r;
    for (usize i = 0; i < n; i++) {
        r[i] = mod(a[i], b[i]);
    }
    return r;
}
template <class TA, class TB, usize n>
constexpr auto rem (const GVec<TA, n>& a, const GVec<TB, n>& b) {
    GVec<decltype(rem(a[0], b[0])), n> r;
    for (usize i = 0; i < n; i++) {
        r[i] = rem(a[i], b[i]);
    }
    return r;
}

 // Dot product of two vectors.
template <class T, usize n>
constexpr auto dot (const GVec<T, n>& a, const GVec<T, n>& b) {
    decltype(wide_multiply(a[0], b[0])) r = 0;
    for (usize i = 0; i < n; i++) {
        r += wide_multiply(a[i], b[i]);
    }
    return r;
}

 // Linearly interpolate between two vectors.
template <class T, usize n>
constexpr GVec<T, n> lerp (
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

 // Cross product of 3-dimensional vectors.
template <class A, class B>
constexpr auto cross (const GVec<A, 3>& a, const GVec<B, 3>& b) {
    return GVec<decltype(A() * B()), 3>{
       a.y * b.z - a.z * b.y,
       a.z * b.x - a.x * b.z,
       a.x * b.y - a.y * b.x
    };
}

} // namespace geo

///// GENERIC AYU DESCRIPTION

AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T, uni::usize n),
    AYU_DESCRIBE_TEMPLATE_TYPE(geo::GVec<T, n>),
    desc::name([]{
        using namespace std::literals;
        using namespace uni;
        if constexpr (std::is_same_v<T, float>) {
            if constexpr (n == 2) { return "geo::Vec"sv; }
            else if constexpr (n == 3) { return "geo::Vec3"sv; }
            else if constexpr (n == 4) { return "geo::Vec4"sv; }
        }
        else if constexpr (std::is_same_v<T, double>) {
            if constexpr (n == 2) { return "geo::DVec"sv; }
            else if constexpr (n == 3) { return "geo::DVec3"sv; }
            else if constexpr (n == 4) { return "geo::DVec4"sv; }
        }
        else if constexpr (std::is_same_v<T, int32>) {
            if constexpr (n == 2) { return "geo::IVec"sv; }
            else if constexpr (n == 3) { return "geo::IVec3"sv; }
            else if constexpr (n == 4) { return "geo::IVec4"sv; }
        }
        else if constexpr (std::is_same_v<T, int64>) {
            if constexpr (n == 2) { return "geo::LVec"sv; }
            else if constexpr (n == 3) { return "geo::LVec3"sv; }
            else if constexpr (n == 4) { return "geo::LVec4"sv; }
        }
        else if constexpr (std::is_same_v<T, bool>) {
            if constexpr (n == 2) { return "geo::BVec"sv; }
            else if constexpr (n == 3) { return "geo::BVec3"sv; }
            else if constexpr (n == 4) { return "geo::BVec4"sv; }
        }
        static std::string r = "geo::GVec<" + std::string(ayu::Type::CppType<T>().name())
                        + ", " + std::to_string(n) + ">";
        return Str(r);
    }),
    desc::length(desc::template constant<uni::usize>(n)),
    desc::elem_func([](geo::GVec<T, n>& v, uni::usize i){
        if (i < n) return ayu::Reference(&v[i]);
        else return ayu::Reference();
    })
)

template <class T, geo::usize n>
struct std::tuple_size<geo::GVec<T, n>> {
    static constexpr geo::usize value = n;
};

template <geo::usize i, class T, geo::usize n>
struct std::tuple_element<i, geo::GVec<T, n>> {
    using type = T;
};


#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"
template <class T, uni::usize n>
struct tap::Show<geo::GVec<T, n>> {
    std::string show (const geo::GVec<T, n>& v) {
        std::string r = "[" + std::to_string(v[0]);
        for (uni::usize i = 1; i < n; i++) {
            r += ", " + std::to_string(v[i]);
        }
        return r + "]";
    }
};
#endif
