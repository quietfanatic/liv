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
struct Traversal;
inline const Traversal* current_traversal = null;
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
        LocationRef location;
         // ATTR, ATTR_FUNC
         // Can't include Str directly because it has non-trivial constructor
        const Str* key;
         // ELEM, ELEM_FUNC
        usize index;
    };
    Traversal() : parent(current_traversal) {
        current_traversal = this;
    }
    ~Traversal() {
        expect(current_traversal == this);
        current_traversal = parent;
    }
};

using TravCallback = Callback<void(const Traversal&)>;

 // If only_addressable is true, will skip over any items that aren't
 // addressable and don't have pass_through_addressable.
static inline void trav_start (
    const Reference& ref, LocationRef loc, bool only_addressable,
    AccessMode mode, TravCallback cb
) {
    Traversal trav;
    trav.address = ref.address();
    trav.desc = DescriptionPrivate::get(ref.type());
    trav.readonly = ref.readonly();
    trav.addressable = !!trav.address;
    trav.children_addressable = !!trav.address ||
        ref.acr->accessor_flags & ACR_PASS_THROUGH_ADDRESSABLE;
    trav.only_addressable = only_addressable;
    trav.op = START;
    trav.reference = &ref;
    trav.location = loc;
    if (trav.address) {
        cb(trav);
    }
    else if (!trav.only_addressable || trav.children_addressable) {
        ref.access(mode, [&](Mu& v){
            trav.address = &v;
            cb(trav);
        });
    }
}

static inline void trav_acr (
    Traversal& trav, const Traversal& parent, const Accessor* acr,
    AccessMode mode, TravCallback cb
) {
    trav.address = acr->address(*parent.address);
    trav.desc = DescriptionPrivate::get(acr->type(parent.address));
    trav.readonly = parent.readonly || acr->accessor_flags & ACR_READONLY;
    if (parent.children_addressable) {
        trav.addressable = !!trav.address;
        trav.children_addressable = !!trav.address ||
            acr->accessor_flags & ACR_PASS_THROUGH_ADDRESSABLE;
    }
    else {
        trav.addressable = trav.children_addressable = false;
    }
    trav.only_addressable = parent.only_addressable;
    trav.acr = acr;
    if (trav.address) {
        cb(trav);
    }
    else if (!trav.only_addressable || trav.children_addressable) {
        acr->access(mode, *parent.address, [&](Mu& v){
            trav.address = &v;
            cb(trav);
        });
    }
}

static inline void trav_ref (
    Traversal& trav, const Traversal& parent, const Reference& ref,
    AccessMode mode, TravCallback cb
) {
    trav.address = ref.address();
    trav.desc = DescriptionPrivate::get(ref.type());
    trav.readonly = parent.readonly || ref.readonly();
    if (parent.children_addressable) {
        trav.addressable = !!trav.address;
        trav.children_addressable = !!trav.address ||
            ref.acr->accessor_flags & ACR_PASS_THROUGH_ADDRESSABLE;
    }
    else {
        trav.addressable = trav.children_addressable = false;
    }
    trav.only_addressable = parent.only_addressable;
    if (trav.address) {
        cb(trav);
    }
    else if (!trav.only_addressable || trav.children_addressable) {
        ref.access(mode, [&](Mu& v){
            trav.address = &v;
            cb(trav);
        });
    }
}

static inline void trav_delegate (
    const Traversal& parent, const Accessor* acr, AccessMode mode, TravCallback cb
) {
    expect(&parent == current_traversal);
    Traversal trav;
    trav.op = DELEGATE;
    trav_acr(trav, parent, acr, mode, cb);
}

static inline void trav_attr (
    const Traversal& parent, const Accessor* acr, const Str& key,
    AccessMode mode, TravCallback cb
) {
    expect(&parent == current_traversal);
    Traversal trav;
    trav.op = ATTR;
    trav.key = &key;
    trav_acr(trav, parent, acr, mode, cb);
}

static inline void trav_attr_func (
    const Traversal& parent, const Reference& ref,
    Reference(* func )(Mu&, Str), const Str& key, AccessMode mode, TravCallback cb
) {
    expect(&parent == current_traversal);
    Traversal trav;
    trav.op = ATTR_FUNC;
    trav.attr_func = func;
    trav.key = &key;
    trav_ref(trav, parent, ref, mode, cb);
}

static inline void trav_elem (
    const Traversal& parent, const Accessor* acr, usize index,
    AccessMode mode, TravCallback cb
) {
    expect(&parent == current_traversal);
    Traversal trav;
    trav.op = ELEM;
    trav.index = index;
    trav_acr(trav, parent, acr, mode, cb);
}

static inline void trav_elem_func (
    const Traversal& parent, const Reference& ref,
    Reference(* func )(Mu&, usize), usize index, AccessMode mode, TravCallback cb
) {
    expect(&parent == current_traversal);
    Traversal trav;
    trav.op = ELEM_FUNC;
    trav.elem_func = func;
    trav.index = index;
    trav_ref(trav, parent, ref, mode, cb);
}

static Reference trav_reference (const Traversal& trav) {
    if (trav.addressable) {
        return Pointer(
            trav.address,
            trav.readonly ? Type(trav.desc).add_readonly() : trav.desc
        );
    }
    else if (trav.op == START) {
        return *trav.reference;
    }
    else {
        Reference parent = trav_reference(*trav.parent);
        switch (trav.op) {
            case DELEGATE: case ATTR: case ELEM:
                return parent.chain(trav.acr);
            case ATTR_FUNC:
                return parent.chain_attr_func(trav.attr_func, *trav.key);
            case ELEM_FUNC:
                return parent.chain_elem_func(trav.elem_func, trav.index);
            default: never();
        }
    }
}

static Location trav_location (const Traversal& trav) {
    if (trav.op == START) {
        return trav.location;
    }
    else {
        Location parent = trav_location(*trav.parent);
        switch (trav.op) {
            case DELEGATE: return parent;
            case ATTR: case ATTR_FUNC:
                return Location(parent, *trav.key);
            case ELEM: case ELEM_FUNC:
                return Location(parent, trav.index);
            default: never();
        }
    }
}

} // namespace ayu::in
