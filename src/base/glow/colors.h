#pragma once

#include "common.h"

namespace glow {

struct RGBA8 {
    uint8 r;
    uint8 g;
    uint8 b;
    uint8 a;
    CE RGBA8 (uint8 r, uint8 g, uint8 b, uint8 a) : r(r), g(g), b(b), a(a) { }
     // Convert from a uint32 in 0xRRGGBBAA format (native endian)
    CE RGBA8 (uint32 rgba = 0) :
        r(rgba >> 24), g(rgba >> 16), b(rgba >> 8), a(rgba)
    { }
    explicit CE operator uint32 () {
        return uint32(r) << 24 | uint32(g) << 16 | uint32(b) << 8 | uint32(a);
    }
};
static bool operator == (RGBA8 a, RGBA8 b) {
    return uint32(a) == uint32(b);
}
static bool operator != (RGBA8 a, RGBA8 b) {
    return uint32(a) != uint32(b);
}

} // namespace glow

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"
namespace tap {
    template <>
    struct Show<glow::RGBA8> {
        std::string show (const glow::RGBA8& v) {
            return "RGBA8("s + std::to_string(v.r)
                     + ", "s + std::to_string(v.g)
                     + ", "s + std::to_string(v.b)
                     + ", "s + std::to_string(v.a)
                     + ")"s;
        }
    };
}
#endif
