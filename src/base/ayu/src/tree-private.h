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

 // Can't be TreeRef because then t.data.as_chars will be invalidated when this
 // function returns.
inline Str tree_shortStr (const Tree& t) {
    expect(t.rep == REP_SHORTSTRING && t.length <= 8);
    return Str(t.data.as_chars, t.length);
}
inline Str tree_longStr (TreeRef t) {
    expect(t->rep == REP_LONGSTRING && t->length > 8);
    return Str(t->data.as_char_ptr, t->length);
}
inline TreeArraySlice tree_Array (TreeRef t) {
    expect(t->rep == REP_ARRAY);
    return TreeArraySlice(t->data.as_array_ptr, t->length);
}
inline TreeArray tree_Array (Tree&& t) {
    expect(t.rep == REP_ARRAY);
    auto r = TreeArray::Materialize(
        (Tree*)t.data.as_array_ptr, t.length
    );
    new (&t) Tree();
    return r;
}
inline TreeObjectSlice tree_Object (TreeRef t) {
    expect(t->rep == REP_OBJECT);
    return TreeObjectSlice(t->data.as_object_ptr, t->length);
}
inline TreeObject tree_Object (Tree&& t) {
    expect(t.rep == REP_OBJECT);
    auto r = TreeObject::Materialize(
        (TreePair*)t.data.as_object_ptr, t.length
    );
    new (&t) Tree();
    return r;
}
inline const std::exception_ptr& tree_Error (TreeRef t) {
    expect(t->rep == REP_ERROR);
    return *t->data.as_error_ptr;
}

} // namespace ayu::in
