#include "dynamic.h"

#include "describe.h"
#include "reference.h"

using namespace hacc;
using namespace hacc::in;

static const Dynamic empty_dynamic;

HACCABLE(hacc::Dynamic,
    values_custom(
        [](const Dynamic& a, const Dynamic& b) -> bool {
            return a.has_value() == b.has_value();
        },
        [](Dynamic& a, const Dynamic& b) { a = const_cast<Dynamic&&>(b); },
        value_pointer(null, &empty_dynamic)
    ),
    length(constant<usize>(2)),
    elems(
        elem(value_funcs<Type>(
            [](const Dynamic& v){ return v.type; },
            [](Dynamic& v, Type t){
                v = Dynamic(t);
            }
        )),
        elem(reference_func([](Dynamic& v){ return Reference(v); }))
    )
)

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"
#include "parse.h"
#include "serialize.h"

namespace hacc::test {
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

    struct NoCopy {
        NoCopy () { }
        NoCopy (const NoCopy&) = delete;
    };

    struct NoDestructor {
        ~NoDestructor () = delete;
    };
} using namespace hacc::test;

 // The things here should work without any descriptions
HACCABLE_0(hacc::test::DynamicTest)
HACCABLE_0(hacc::test::Test2)
HACCABLE_0(hacc::test::NoConstructor)
HACCABLE_0(hacc::test::NoCopy)
HACCABLE_0(hacc::test::NoDestructor)

static tap::TestSet tests ("base/hacc/dynamic", []{
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
    throws<X::CannotCoerce>([&]{ d.as<bool>(); }, "X::CannotCoerce");
    throws<X::CannotDefaultConstruct>([&]{
        Dynamic(Type::CppType<NoConstructor>());
    }, "X::CannotDefaultConstruct");
    throws<X::CannotDestruct>([&]{
        d = Dynamic(Type::CppType<NoDestructor>());
    }, "Cannot construct type without destructor");

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

    done_testing();
});
#endif
