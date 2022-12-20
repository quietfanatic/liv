#include "colors.h"

#include "../ayu/describe.h"

using namespace glow;

AYU_DESCRIBE(glow::RGBA8,
    to_tree([](const RGBA8& v, ayu::TreeFlags){
        return ayu::Tree(uint32(v), ayu::PREFER_HEX);
    }),
    from_tree([](RGBA8& v, const ayu::Tree& t){
        if (t.form == ayu::NUMBER) {
            v = RGBA8(int32(t));
        }
        else if (t.form == ayu::ARRAY) {
            auto& a = static_cast<const ayu::Array&>(t);
            if (a.size() != 4) {
                throw ayu::X<ayu::WrongLength>(
                    ayu::current_location(), 4u, 4u, a.size()
                );
            }
            v = RGBA8(uint8(a[0]), uint8(a[1]), uint8(a[2]), uint8(a[3]));
        }
        else throw ayu::X<ayu::InvalidForm>(ayu::current_location(), t);
    })
)
