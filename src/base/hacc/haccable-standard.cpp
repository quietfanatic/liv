// Provides haccabilities for builtin scalar types.  For template types like
// std::vector, include the .h file.

#include "haccable-standard.h"

using namespace hacc;

#define SCALAR_HACCABLE(type) \
HACCABLE(type, \
    to_tree([](const type& v){ return Tree(v); }), \
    from_tree([](type& v, const Tree& t){ v = type(t); }) \
)

SCALAR_HACCABLE(std::nullptr_t)
 // Even though these are in hacc::, serialize them without the prefix.
SCALAR_HACCABLE(bool)
SCALAR_HACCABLE(char)
SCALAR_HACCABLE(int8)
SCALAR_HACCABLE(uint8)
SCALAR_HACCABLE(int16)
SCALAR_HACCABLE(uint16)
SCALAR_HACCABLE(int32)
SCALAR_HACCABLE(uint32)
SCALAR_HACCABLE(int64)
SCALAR_HACCABLE(uint64)
SCALAR_HACCABLE(float)
SCALAR_HACCABLE(double)
 // For now, haccable std::string but not other std::basic_string types.
HACCABLE(std::string,
    to_tree([](const std::string& v){ return Tree(v); }),
    from_tree([](std::string& v, const Tree& t){ v = std::string(Str(t)); })
)
 // Str and const char* are not haccable because they're reference types.

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"

static tap::TestSet tests ("base/hacc/haccable-standard", []{
    using namespace tap;
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
        got_s = item_to_string(&expected_data, hacc::COMPACT);
    }, "item_to_string on tuple");
    is(got_s, s, "gives correct result");
    done_testing();
});

#endif
