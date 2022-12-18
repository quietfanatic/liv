// This provides special generic values for NAN and INF.

#pragma once

#include <limits>
#include "common.h"
#include "type_traits.h"

namespace geo {

 // Represents not-a-number, or an undefined number.  Only representable by
 // floating point types or things that contain them.
struct GNAN_t {
     // Only define coercion to one floating point type to avoid ambiguous
     // overloads.
    CE operator float () const {
        return std::numeric_limits<float>::quiet_NaN();
    }
     // Explicitly forbid coercion to non-floating-point types
    template <class T> requires (std::is_integral_v<T>)
    CE operator T () const {
        static_assert((T*)nullptr, "Cannot coerce GNAN to integer type");
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

 // Comparisons directly with GINF to avoid ambiguous conversions.  There is no
 // equivalent comparison with GNAN because comparing with GNAN always returns
 // false.

#define GINF_COMPARISON(op) \
template <class T> \
CE bool operator op (GINF_t a, const T& b) { \
    return T(a) op b; \
} \
template <class T> \
CE bool operator op (const T& a, GINF_t b) { \
    return a op T(b); \
}
GINF_COMPARISON(==)
GINF_COMPARISON(!=)
GINF_COMPARISON(<)
GINF_COMPARISON(<=)
GINF_COMPARISON(>=)
GINF_COMPARISON(>)
#undef GINF_COMPARISON

} // namespace geo
