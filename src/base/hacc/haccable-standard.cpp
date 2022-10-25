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
 // Str and const char* are not haccable since their ownership is ambiguous.

