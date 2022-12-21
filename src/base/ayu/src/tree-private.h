#pragma once

#include "../tree.h"

#include "../../tap/tap.h"
#include "../exception.h"
#include "../print.h"

namespace ayu::in {

using TreeRep = int8;
enum : TreeRep {
    REP_UNDEFINED,
    REP_NULL,
    REP_BOOL,
    REP_INT64,
    REP_DOUBLE,
     // Short string's length is encoded here
    REP_0CHARS,
    REP_1CHARS,
    REP_2CHARS,
    REP_3CHARS,
    REP_4CHARS,
    REP_5CHARS,
    REP_6CHARS,
    REP_7CHARS,
    REP_8CHARS,
     // Types requiring reference counting
    REP_VARCHAR = -1,
    REP_ARRAY = -2,
    REP_OBJECT = -3,
    REP_ERROR = -4,
};

template <class T>
struct TreeData : RefCounted {
    T value;
};

struct VarChar {
    usize size;
    char data[0];
};

inline Str tree_shortStr (const Tree& t) {
    assert(t.rep >= REP_0CHARS && t.rep <= REP_8CHARS);
    return Str(t.data.as_chars, t.rep - REP_0CHARS);
}
inline Str tree_longStr (const Tree& t) {
    assert(t.rep == REP_VARCHAR);
    auto& vc = ((const TreeData<VarChar>*)t.data.as_ptr)->value;
    assert(vc.size > 8);
    return Str(vc.data, vc.size);
}
inline const Array& tree_Array (const Tree& t) {
    assert(t.rep == REP_ARRAY);
    return ((const TreeData<Array>*)t.data.as_ptr)->value;
}
inline const Object& tree_Object (const Tree& t) {
    assert(t.rep == REP_OBJECT);
    return ((const TreeData<Object>*)t.data.as_ptr)->value;
}
inline const std::exception_ptr& tree_Error (const Tree& t) {
    assert(t.rep == REP_ERROR);
    return ((const TreeData<std::exception_ptr>*)t.data.as_ptr)->value;
}

} // namespace ayu::in
