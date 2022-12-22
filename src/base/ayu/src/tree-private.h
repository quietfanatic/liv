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
    REP_SHORTSTRING,
     // Types requiring reference counting
    REP_LONGSTRING = -1,
    REP_ARRAY = -2,
    REP_OBJECT = -3,
    REP_ERROR = -4,
};

template <class T>
struct TreeData : RefCounted {
    alignas(uint64) T value;
};

inline Str tree_shortStr (const Tree& t) {
    assert(t.rep == REP_SHORTSTRING && t.length <= 8);
    return Str(t.data.as_chars, t.length);
}
inline Str tree_longStr (const Tree& t) {
    assert(t.rep == REP_LONGSTRING && t.length > 8);
    const char* vc = ((const TreeData<char[0]>*)t.data.as_ptr)->value;
    return Str(vc, t.length);
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
