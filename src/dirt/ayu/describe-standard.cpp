// Provides ayu descriptions for builtin scalar types.  For template types like
// std::vector, include the .h file.

#include "../uni/iri.h"
#include "describe-standard.h"
#include "resource.h"

using namespace ayu;

#define AYU_DESCRIBE_SCALAR(type) \
AYU_DESCRIBE(type, \
    to_tree([](const type& v){ return Tree(v); }), \
    from_tree([](type& v, const Tree& t){ v = type(t); }) \
)

AYU_DESCRIBE_SCALAR(std::nullptr_t)
AYU_DESCRIBE_SCALAR(bool)
AYU_DESCRIBE_SCALAR(char)
 // Even though these are in ayu::, serialize them without the namespace.
AYU_DESCRIBE_SCALAR(int8)
AYU_DESCRIBE_SCALAR(uint8)
AYU_DESCRIBE_SCALAR(int16)
AYU_DESCRIBE_SCALAR(uint16)
AYU_DESCRIBE_SCALAR(int32)
AYU_DESCRIBE_SCALAR(uint32)
AYU_DESCRIBE_SCALAR(int64)
AYU_DESCRIBE_SCALAR(uint64)
AYU_DESCRIBE_SCALAR(float)
AYU_DESCRIBE_SCALAR(double)
AYU_DESCRIBE_SCALAR(uni::AnyString)
#undef AYU_DESCRIBE_SCALAR
AYU_DESCRIBE(uni::UniqueString,
    to_tree([](const UniqueString& v){ return Tree(AnyString(v)); }),
    from_tree([](UniqueString& v, const Tree& t){ v = AnyString(t); })
)
AYU_DESCRIBE(uni::SharedString,
    to_tree([](const SharedString& v){ return Tree(AnyString(v)); }),
    from_tree([](SharedString& v, const Tree& t){ v = AnyString(t); })
)
AYU_DESCRIBE(std::string,
    to_tree([](const std::string& v){ return Tree(Str(v)); }),
    from_tree([](std::string& v, const Tree& t){ v = std::string(Str(t)); })
)
AYU_DESCRIBE(std::u16string,
    to_tree([](const std::u16string& v){ return Tree(Str16(v)); }),
     // Inefficient but I don't really care about std::u16string, I'm just using
     // it for testing
    from_tree([](std::u16string& v, const Tree& t){ v = UniqueString16(t); })
)

 // string_view is a reference-like type so it can't be deserialized because the
 // data structure containing it would most likely outlive the tree it came
 // from.  However, allowing it to be serialized is useful for error messages.
AYU_DESCRIBE(std::string_view,
    to_tree([](const std::string_view& v){
        return Tree(Str(v));
    })
)
AYU_DESCRIBE(uni::Str,
    to_tree([](const Str& v){
        return Tree(v);
    })
)
AYU_DESCRIBE(uni::StaticString,
    to_tree([](const StaticString& v){
        return Tree(v);
    })
)
 // We can't do the same for const char* because the full specialization would
 // conflict with the partial specialization for T*.  Normally this wouldn't be
 // a problem, but if the templates are in different compilation units, it'll
 // cause a duplicate definition error from the linker.

AYU_DESCRIBE(uni::IRI,
    delegate(mixed_funcs<AnyString>(
        [](const IRI& v) {
             // current_location().as_iri() would be more intentful, but also
             // slower since it does a bunch of string concatenation to build
             // the IRI fragment, which we don't need.
            if (auto res = current_location().root_resource()) {
                return AnyString(v.spec_relative_to(res->name()));
            }
            else return v.spec();
        },
        [](IRI& v, const AnyString& s){
            if (s.empty()) {
                v = iri::IRI();
            }
            else {
                if (auto res = current_location().root_resource()) {
                    v = iri::IRI(s, res->name());
                }
                else {
                    v = iri::IRI(s);
                }
                if (!v) throw ayu::X<GenericError>(cat("Invalid IRI ", s));
            }
        }
    ))
)

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"

static tap::TestSet tests ("dirt/ayu/describe-standard", []{
    using namespace tap;
     // Test wstrings
    std::string s8 = "\"あいうえお\""s;
    std::u16string s16 = u"あいうえお"s;
    is(item_to_string(&s16), s8, "Can serialize wstring");
    std::u16string s16_got;
    doesnt_throw([&]{
        item_from_string(&s16_got, s8);
    });
    is(s16_got, s16, "Can deserialize wstring");
     // Test tuples
    std::tuple<int32, std::string, std::vector<int32>> data;
    std::tuple<int32, std::string, std::vector<int32>> expected_data
        = {45, "asdf"s, {3, 4, 5}};
    Str s = "[45 asdf [3 4 5]]";
    doesnt_throw([&]{
        return item_from_string(&data, s);
    }, "item_from_string on tuple");
    is(data, expected_data, "gives correct result");
    std::string got_s;
    doesnt_throw([&]{
        got_s = item_to_string(&expected_data);
    }, "item_to_string on tuple");
    is(got_s, s, "gives correct result");
     // Test uni arrays
    uni::AnyArray<uni::AnyString> strings {
        "asdf", "fdsa", "foo", "bar"
    };
    StaticString strings_s = "[asdf fdsa foo bar]";
    is(item_to_string(&strings), strings_s, "uni arrays and strings");
    done_testing();
});

#endif
