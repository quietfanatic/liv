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
enum TraversalOp : uint8 {
    START,
    DELEGATE,
    ATTR,
    ATTR_FUNC,
    ELEM,
    ELEM_FUNC,
};
struct Traversal {
    const Traversal* parent;
    Mu* address;
    const DescriptionPrivate* desc;
     // Type can keep track of readonly, but DescriptionPrivate* can't, so keep
     // track of it here.
    bool readonly;
     // If this item has a stable address, then trav_reference() can use the
     // address directly instead of having to chain from parent.
    bool addressable;
     // Set if this item has pass_through_addressable AND parent->addressable is
     // true.
    bool children_addressable;
     // Only traverse addressable items.  If an unaddressable and
     // non-pass-through item is encountered, the traversal's callback will not
     // be called.
    bool only_addressable;
    TraversalOp op;
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
     // Just updates current_traversal
    Traversal();
    ~Traversal();
};

inline const Traversal* current_traversal = null;

using TravCallback = Callback<void(const Traversal&)>;

 // If only_addressable is true, will skip over any items that aren't
 // addressable and don't have pass_through_addressable.
void trav_start (
    const Reference&, const Location&, bool only_addressable,
    AccessMode, TravCallback
);
void trav_delegate (const Traversal&, const Accessor*, AccessMode, TravCallback);
void trav_attr (
    const Traversal&, const Accessor*, const Str&, AccessMode, TravCallback
);
void trav_attr_func (
    const Traversal&, const Reference&, Reference(*)(Mu&, Str), const Str&,
    AccessMode, TravCallback
);
void trav_elem (
    const Traversal&, const Accessor*, usize, AccessMode, TravCallback
);
void trav_elem_func (
    const Traversal&, const Reference&, Reference(*)(Mu&, usize), usize,
    AccessMode, TravCallback
);

Reference trav_reference (const Traversal&);
Location trav_location (const Traversal&);

} // namespace ayu::in
