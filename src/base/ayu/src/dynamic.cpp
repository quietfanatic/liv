#include "../dynamic.h"

#include "../describe.h"
#include "../reference.h"

using namespace ayu;
using namespace ayu::in;

static const Dynamic empty_dynamic;

AYU_DESCRIBE(ayu::Dynamic,
    values_custom(
        [](const Dynamic& a, const Dynamic& b) -> bool {
            return a.has_value() == b.has_value();
        },
        [](Dynamic& a, const Dynamic& b) { a = const_cast<Dynamic&&>(b); },
        value_pointer(null, &empty_dynamic)
    ),
    elems(
        elem(value_funcs<Type>(
            [](const Dynamic& v){ return v.type; },
            [](Dynamic& v, Type t){
                v = Dynamic(t);
            }
        )),
        elem(reference_func([](Dynamic& v){ return Reference(v.ptr()); }))
    )
)

#ifndef TAP_DISABLE_TESTS
#include "../../tap/tap.h"
#include "../parse.h"
#include "../serialize.h"

namespace ayu::test {
    struct DynamicTest {
        int a;
        int b;
    };

    struct Test2 {
        int a;
    };
    bool operator == (const Test2& a, const Test2& b) {
        return a.a == b.a;
    }

    struct NoConstructor {
        NoConstructor () = delete;
    };
    struct CustomConstructor {
        CustomConstructor () = delete;
        ~CustomConstructor () = delete;
    };

    struct NoCopy {
        NoCopy () { }
        NoCopy (const NoCopy&) = delete;
    };

    struct NoDestructor {
        ~NoDestructor () = delete;
    };
    struct alignas(256) WeirdAlign {
        WeirdAlign () {
            if (reinterpret_cast<usize>(this) & (256-1)) {
                throw std::runtime_error("Aligned allocation didn't work");
            }
        }
    };
} using namespace ayu::test;

 // The things here should work without any descriptions
AYU_DESCRIBE(ayu::test::DynamicTest)
AYU_DESCRIBE(ayu::test::Test2)
AYU_DESCRIBE(ayu::test::NoConstructor)
AYU_DESCRIBE(ayu::test::NoCopy)
AYU_DESCRIBE(ayu::test::NoDestructor)
AYU_DESCRIBE(ayu::test::WeirdAlign)

AYU_DESCRIBE(ayu::test::CustomConstructor,
    default_construct([](void*){ }),
    destroy([](CustomConstructor*){ })
)

static tap::TestSet tests ("base/ayu/dynamic", []{
    using namespace tap;
    Dynamic d;
    ok(!d.has_value(), "Default Dynamic::has_value is false");
    d = true;
    ok(d.as<bool>(), "Can make Dynamic bool w/ implicit coercions");
    d = false;
    ok(!d.as<bool>(), "Can make Dynamic false bool w/ implicit coercions");
    ok(d.has_value(), "Dynamic false bool is has_value");
    d = DynamicTest{4, 5};
    is(d.as<DynamicTest>().b, 5, "Can make Dynamic with struct type");
    throws<CannotCoerce>([&]{ d.as<bool>(); }, "CannotCoerce");
    throws<CannotDefaultConstruct>([&]{
        Dynamic(Type::CppType<NoConstructor>());
    }, "CannotDefaultConstruct");
    throws<CannotDestroy>([&]{
        d = Dynamic(Type::CppType<NoDestructor>());
    }, "Cannot construct type without destructor");

    doesnt_throw([&]{
        d = Dynamic(Type::CppType<CustomConstructor>());
    }, "Can construct type with externally-supplied constructor/destructor");

    d = int32(4);
    is(item_to_tree(&d), tree_from_string("[int32 4]"), "Dynamic to_tree works");
    doesnt_throw([&]{
        item_from_string(&d, "[double 55]");
    });
    is(d.type, Type::CppType<double>(), "Dynamic from_tree gives correct type");
    is(d.as<double>(), double(55), "Dynamic from_tree gives correct value");
    doesnt_throw([&]{
        item_from_string(&d, "null");
    });
    ok(!d.has_value(), "Dynamic from_tree with null makes unhas_value Dynamic");
    doesnt_throw([&]{
        Dynamic::make<WeirdAlign>();
    }, "Can allocate object with non-standard alignment");

    done_testing();
});
#endif
