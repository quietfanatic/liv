// Provides ayu descriptions for builtin scalar types.  For template types like
// std::vector, include the .h file.

#include "../iri/iri.h"
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
#undef AYU_DESCRIBE_SCALAR

AYU_DESCRIBE(std::string,
    to_tree([](const std::string& v){ return Tree(v); }),
    from_tree([](std::string& v, const Tree& t){ v = std::string(t); })
)
AYU_DESCRIBE(std::u16string,
    to_tree([](const std::u16string& v){ return Tree(v); }),
    from_tree([](std::u16string& v, const Tree& t){ v = std::u16string(t); })
)
 // Str and const char* are not describable because they're reference types, so
 // their ownership is ambiguous.

AYU_DESCRIBE(iri::IRI,
    delegate(mixed_funcs<String>(
        [](const iri::IRI& v) {
            if (auto res = current_resource()) {
                return v.spec_relative_to(res.name());
            }
            else return v.spec();
        },
        [](iri::IRI& v, const String& s){
            if (s.empty()) {
                v = iri::IRI();
            }
            else {
                if (auto res = current_resource()) {
                    v = iri::IRI(s, res.name());
                }
                else {
                    v = iri::IRI(s);
                }
                if (!v) throw X::GenericError("Invalid IRI " + s);
            }
        }
    ))
)

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"

static tap::TestSet tests ("base/ayu/describe-standard", []{
    using namespace tap;
     // Test wstrings
    std::string s8 = "\"あいうえお\""s;
    std::u16string s16 = u"あいうえお"s;
    is(item_to_string(&s16, COMPACT), s8, "Can serialize wstring");
    std::u16string s16_got;
    doesnt_throw([&]{
        item_from_string(&s16_got, s8);
    });
    is(s16_got, s16, "Can deserialize wstring");
     // Test tuples
    std::tuple<int32, String, std::vector<int32>> data;
    std::tuple<int32, String, std::vector<int32>> expected_data
        = {45, "asdf"s, {3, 4, 5}};
    Str s = "[45 asdf [3 4 5]]"sv;
    doesnt_throw([&]{
        return item_from_string(&data, s);
    }, "item_from_string on tuple");
    is(data, expected_data, "gives correct result");
    String got_s;
    doesnt_throw([&]{
        got_s = item_to_string(&expected_data, ayu::COMPACT);
    }, "item_to_string on tuple");
    is(got_s, s, "gives correct result");
    done_testing();
});

#endif
