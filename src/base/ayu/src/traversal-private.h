#pragma once

#include "../common.h"
#include "../location.h"
#include "accessors-private.h"
#include "descriptors-private.h"

namespace ayu::in {

 // This tracks the decisions that were made during a serialization operation.
 // It has two purposes:
 //   1. Allow creating a Reference to the current item in case the current item
 //     is not addressable, without having to start over from the very beginning
 //     or duplicate work.  This is mainly to support swizzle and init ops.
 //   2. Track the current location without any heap allocations, but allow
 //     getting an actual heap-allocated Location to the current item if needed
 //     for error reporting.
enum TraversalType : uint8 {
    START,
    DELEGATE,
    ATTR,
    ATTR_FUNC,
    ELEM,
    ELEM_FUNC,
};
struct Traversal {
    const Traversal* parent;
    const DescriptionPrivate* desc;
    Mu* item;
     // If this item has a stable address, then trav_reference() can use the
     // address directly instead of having to chain from parent.
    bool addressable;
    bool readonly;
    TraversalType type;
    union {
         // START
        const Reference* reference;
         // DELEGATE, ATTR, ELEM
        const Accessor* acr;
         // ATTR_FUNC
        Reference(* attr_func )(Mu&, Str);
         // ELEM_FUNC
        Reference(* elem_func )(Mu&, usize);
    };
    union {
         // START
        const Location* location;
         // ATTR, ATTR_FUNC
         // Can't include Str directly because it has non-trivial constructor
        const Str* key;
         // ELEM, ELEM_FUNC
        usize index;
    };
};

using TravCallback = Callback<void(const Traversal&)>;

void trav_start (const Reference&, const Location&, AccessOp, TravCallback);
void trav_delegate (const Traversal&, const Accessor*, AccessOp, TravCallback);
void trav_attr (
    const Traversal&, const Accessor*, const Str&, AccessOp, TravCallback
);
void trav_attr_func (
    const Traversal&, Reference, Reference(*)(Mu&, Str), const Str&,
    AccessOp, TravCallback
);
void trav_elem (
    const Traversal&, const Accessor*, usize, AccessOp, TravCallback
);
void trav_elem_func (
    const Traversal&, Reference, Reference(*)(Mu&, usize), usize,
    AccessOp, TravCallback
);

Reference trav_reference (const Traversal&);
Location trav_location (const Traversal&);

} // namespace ayu::in
