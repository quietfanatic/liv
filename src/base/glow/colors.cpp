#include "colors.h"

#include "../ayu/describe.h"

using namespace glow;

AYU_DESCRIBE(glow::RGBA8,
    to_tree([](const RGBA8& v){
         // TODO: Allow specifying preference for hexadecimal in Tree
        return ayu::Tree(ayu::Array{
            ayu::Tree(v.r),
            ayu::Tree(v.g),
            ayu::Tree(v.b),
            ayu::Tree(v.a)
        });
    }),
    from_tree([](RGBA8& v, const ayu::Tree& t){
        if (t.form() == ayu::NUMBER) {
            v = RGBA8(int32(t));
        }
        else if (t.form() == ayu::ARRAY) {
            auto& a = static_cast<const ayu::Array&>(t);
            if (a.size() != 4) {
                throw ayu::X::WrongLength(ayu::current_location(), 4, 4, a.size());
            }
            v = RGBA8(uint8(a[0]), uint8(a[1]), uint8(a[2]), uint8(a[3]));
        }
        else throw ayu::X::InvalidForm(ayu::current_location(), t);
    })
)
