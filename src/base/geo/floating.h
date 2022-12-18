// Utilities for dealing with floating point numbers.
// You probably don't want to include this directly, include scalar.h instead.

#pragma once

#include <bit>
#include <limits>
#include "common.h"
#include "values.h"

namespace geo {

constexpr uint32 FLOAT_SIGN_BIT = 0x8000'0000;
constexpr uint32 FLOAT_EXPONENT_MASK = 0x7f80'0000;
constexpr uint64 DOUBLE_SIGN_BIT = 0x8000'0000'0000'0000;
constexpr uint64 DOUBLE_EXPONENT_MASK = 0x7ff0'0000'0000'0000;

 // This library only works with standard floating point representations.
static_assert(
    std::bit_cast<uint32>(std::numeric_limits<float>::max()) ==
    FLOAT_EXPONENT_MASK - 1
);
static_assert(
    std::bit_cast<uint32>(std::numeric_limits<float>::lowest()) ==
    (FLOAT_SIGN_BIT | (FLOAT_EXPONENT_MASK - 1))
);
static_assert(
    std::bit_cast<uint32>(std::numeric_limits<float>::denorm_min()) == 1
);
static_assert(
    std::bit_cast<uint64>(std::numeric_limits<double>::max()) ==
    DOUBLE_EXPONENT_MASK - 1
);
static_assert(
    std::bit_cast<uint64>(std::numeric_limits<double>::lowest()) ==
    (DOUBLE_SIGN_BIT | (DOUBLE_EXPONENT_MASK - 1))
);
static_assert(
    std::bit_cast<uint64>(std::numeric_limits<double>::denorm_min()) == 1
);

///// PROPERTIES

CE bool defined (float a) {
    return a == a;
}
CE bool defined (double a) {
    return a == a;
}

CE bool finite (float a) {
     // A full exponent mask means the number is not finite.
    return (std::bit_cast<uint32>(a) & FLOAT_EXPONENT_MASK)
        != FLOAT_EXPONENT_MASK;
}
CE bool finite (double a) {
    return (std::bit_cast<uint64>(a) & DOUBLE_EXPONENT_MASK)
        != DOUBLE_EXPONENT_MASK;
}

CE float length2 (float v) { return v * v; }
CE double length2 (double v) { return v * v; }
 // Okay, I admit, I just wanted a constexpr abs
CE float length (float v) { return v >= 0 ? v : -v; }
CE double length (double v) { return v >= 0 ? v : -v; }

///// MODIFIERS

 // Round toward 0
CE int32 trunc (float a) {
    DA(a >= int32(-GINF) && a <= int32(GINF));
    return int32(a);
}
CE int64 trunc (double a) {
    DA(a >= int32(-GINF) && a <= int32(GINF));
    return int64(a);
}

 // 0.5 => 1, -0.5 => -1
CE int32 round (float a) {
    if (a >= 0) return trunc(a + 0.5f);
    else return trunc(a - 0.5f);
}
CE int64 round (double a) {
    if (a >= 0) return trunc(a + 0.5);
    else return trunc(a - 0.5);
}

CE int32 floor (float a) {
    if (a >= 0) return trunc(a);
    else return int32(-GINF) - trunc(int32(-GINF) - a);
}
CE int64 floor (double a) {
    if (a >= 0) return int64(a);
    else return int64(-GINF) - trunc(int64(-GINF) - a);
}

CE int32 ceil (float a) {
    if (a > 0) return int32(GINF) - trunc(int32(GINF) - a);
    else return trunc(a);
}
CE int64 ceil (double a) {
    if (a > 0) return int64(GINF) - trunc(int64(GINF) - a);
    else return trunc(a);
}

CE float next_quantum (float v) {
    if (!finite(v)) return v;
    uint32 rep = std::bit_cast<uint32>(v);
    if (rep == FLOAT_SIGN_BIT) {
         // Treat minus zero as zero
        return std::bit_cast<float>(1);
    }
    else if (rep == FLOAT_EXPONENT_MASK - 1) {
         // Largest finite number
        return GINF;
    }
    else if (rep & FLOAT_SIGN_BIT) {
        return std::bit_cast<float>(rep - 1);
    }
    else {
        return std::bit_cast<float>(rep + 1);
    }
}
CE double next_quantum (double v) {
    if (!finite(v)) return v;
    uint64 rep = std::bit_cast<uint64>(v);
    if (rep == DOUBLE_SIGN_BIT) {
         // Treat minus zero as zero
        return std::bit_cast<double>(uint64(1));
    }
    else if (rep == DOUBLE_EXPONENT_MASK - 1) {
         // Largest finite number
        return GINF;
    }
    else if (rep & DOUBLE_SIGN_BIT) {
        return std::bit_cast<double>(rep - 1);
    }
    else {
        return std::bit_cast<double>(rep + 1);
    }
}
CE float prev_quantum (float v) {
    if (!finite(v)) return v;
    uint32 rep = std::bit_cast<uint32>(v);
    if (rep == 0) {
         // Skip minus zero
        return std::bit_cast<float>(FLOAT_SIGN_BIT | 1);
    }
    else if (rep == (FLOAT_SIGN_BIT | (FLOAT_EXPONENT_MASK - 1))) {
         // Smallest finite number
        return -GINF;
    }
    else if (rep & FLOAT_SIGN_BIT) {
        return std::bit_cast<float>(rep + 1);
    }
    else {
        return std::bit_cast<float>(rep - 1);
    }
}
CE double prev_quantum (double v) {
    if (!finite(v)) return v;
    uint64 rep = std::bit_cast<uint64>(v);
    if (rep == 0) {
         // Skip minus zero
        return std::bit_cast<double>(DOUBLE_SIGN_BIT | 1);
    }
    else if (rep == (DOUBLE_SIGN_BIT | (DOUBLE_EXPONENT_MASK - 1))) {
         // Smallest finite number
        return -GINF;
    }
    else if (rep & DOUBLE_SIGN_BIT) {
        return std::bit_cast<double>(rep + 1);
    }
    else {
        return std::bit_cast<double>(rep - 1);
    }
}

 // AKA sign for scalars
 // (Can't use (v > 0) - (v < 0) because it converts NAN to 0)
CE float normalize (float v) { return v > 0 ? 1 : v < 0 ? -1 : v; }
CE double normalize (double v) { return v > 0 ? 1 : v < 0 ? -1 : v; }

///// COMBINERS

 // These will not work if a / b is inordinately large.
CE float mod (float a, float b) {
    float ratio = a / b;
    if (ratio >= int32(-GINF) && ratio <= int32(GINF)) {
        return a - trunc(ratio) * b;
    }
    else return GNAN;
}
CE double mod (double a, double b) {
    double ratio = a / b;
    if (ratio >= int32(-GINF) && ratio <= int32(GINF)) {
        return a - trunc(ratio) * b;
    }
    else return GNAN;
}

 // Like mod but sign is always sign of b
CE float rem (float a, float b) {
    float ratio = a / b;
    if (ratio >= int32(-GINF) && ratio <= int32(GINF)) {
        return a - floor(ratio) * b;
    }
    else return GNAN;
}
CE double rem (double a, double b) {
    double ratio = a / b;
    if (ratio >= int32(-GINF) && ratio <= int32(GINF)) {
        return a - floor(ratio) * b;
    }
    else return GNAN;
}

 // AKA copysign for scalars
CE float align (float a, float b) {
    return b >= 0 ? length(a) : -length(a);
}
CE double align (double a, double b) {
    return b >= 0 ? length(a) : -length(a);
}

CE float lerp (float a, float b, float t) {
    return (1-t)*a + t*b;
}
CE double lerp (double a, double b, double t) {
    return (1-t)*a + t*b;
}

} // namespace geo
