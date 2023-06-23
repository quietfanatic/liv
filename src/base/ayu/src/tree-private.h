#pragma once

#include "../tree.h"

#include "../../tap/tap.h"
#include "../exception.h"
#include "../print.h"

namespace ayu::in {

using TreeRep = int8;
enum : TreeRep {
    REP_UNDEFINED = 0,
    REP_NULL = 1,
    REP_BOOL = 2,
    REP_INT64 = 3,
    REP_DOUBLE = 4,
    REP_STATICSTRING = 5,
     // Types requiring reference counting
    REP_SHAREDSTRING = -1,
    REP_ARRAY = -2,
    REP_OBJECT = -3,
    REP_ERROR = -4,
};

inline StaticString tree_StaticString (TreeRef t) {
    expect(t->rep == REP_STATICSTRING);
    return StaticString::Static(t->data.as_char_ptr, t->length);
}
inline SharedString tree_SharedString (TreeRef t) {
    expect(t->rep == REP_SHAREDSTRING);
    if (t->data.as_char_ptr) {
        ++ArrayOwnedHeader::get(t->data.as_char_ptr)->ref_count;
    }
    auto r = SharedString::Materialize(
        const_cast<char*>(t->data.as_char_ptr), t->length
    );
    return r;
}
inline SharedString tree_SharedString (Tree&& t) {
    expect(t.rep == REP_SHAREDSTRING);
    auto r = SharedString::Materialize(
        const_cast<char*>(t.data.as_char_ptr), t.length
    );
    new (&t) Tree();
    return r;
}
inline TreeArraySlice tree_Array (TreeRef t) {
    expect(t->rep == REP_ARRAY);
    return TreeArraySlice(t->data.as_array_ptr, t->length);
}
inline TreeArray tree_Array (Tree&& t) {
    expect(t.rep == REP_ARRAY);
    auto r = TreeArray::Materialize(
        const_cast<Tree*>(t.data.as_array_ptr), t.length
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
        const_cast<TreePair*>(t.data.as_object_ptr), t.length
    );
    new (&t) Tree();
    return r;
}
inline const std::exception_ptr& tree_Error (TreeRef t) {
    expect(t->rep == REP_ERROR);
    return *t->data.as_error_ptr;
}

} // namespace ayu::in
