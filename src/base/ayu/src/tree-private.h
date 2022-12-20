#pragma once

#include "../tree.h"

#include "../../tap/tap.h"
#include "../exception.h"
#include "../print.h"

namespace ayu::in {

using TreeRep = uint8;
enum : TreeRep {
    REP_UNDEFINED,
    REP_NULL,
    REP_BOOL,
    REP_INT64,
    REP_DOUBLE,
    REP_STRING,
    REP_ARRAY,
    REP_OBJECT,
    REP_ERROR,
    REP_0CHARS,
    REP_1CHARS,
    REP_2CHARS,
    REP_3CHARS,
    REP_4CHARS,
    REP_5CHARS,
    REP_6CHARS,
    REP_7CHARS,
    REP_8CHARS,
};

template <class T>
struct TreeData : RefCounted {
    T value;
};

inline Str tree_chars (const Tree& t) {
    return Str(t.data.as_chars, t.rep - REP_0CHARS);
}
inline const String& tree_String (const Tree& t) {
    return ((const TreeData<String>*)t.data.as_ptr)->value;
}
inline const Array& tree_Array (const Tree& t) {
    return ((const TreeData<Array>*)t.data.as_ptr)->value;
}
inline const Object& tree_Object (const Tree& t) {
    return ((const TreeData<Object>*)t.data.as_ptr)->value;
}
inline const std::exception_ptr& tree_Error (const Tree& t) {
    return ((const TreeData<std::exception_ptr>*)t.data.as_ptr)->value;
}

} // namespace ayu::in
