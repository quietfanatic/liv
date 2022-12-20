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
    REP_ERROR
};

template <class T>
struct TreeData : RefCounted {
    T value;
};

inline bool tree_bool (const Tree& t) { return t.data.as_usize; }
inline int64 tree_int64 (const Tree& t) { return t.data.as_int64; }
inline double tree_double (const Tree& t) { return t.data.as_double; }
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
