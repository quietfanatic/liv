#include "traversal-private.h"

#include "../reference.h"

namespace ayu::in {

Traversal::Traversal () : parent(current_traversal) {
    current_traversal = this;
}
Traversal::~Traversal () {
    assert(current_traversal == this);
    current_traversal = parent;
}

void trav_start (
    const Reference& ref, const Location& loc, bool only_addressable,
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
    trav.location = &loc;
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

static void trav_acr (
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

static void trav_ref (
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

void trav_delegate (
    const Traversal& parent, const Accessor* acr, AccessMode mode, TravCallback cb
) {
    assert(&parent == current_traversal);
    Traversal trav;
    trav.op = DELEGATE;
    trav_acr(trav, parent, acr, mode, cb);
}

void trav_attr (
    const Traversal& parent, const Accessor* acr, const Str& key,
    AccessMode mode, TravCallback cb
) {
    assert(&parent == current_traversal);
    Traversal trav;
    trav.op = ATTR;
    trav.key = &key;
    trav_acr(trav, parent, acr, mode, cb);
}

void trav_attr_func (
    const Traversal& parent, const Reference& ref,
    Reference(* func )(Mu&, Str), const Str& key, AccessMode mode, TravCallback cb
) {
    assert(&parent == current_traversal);
    Traversal trav;
    trav.op = ATTR_FUNC;
    trav.attr_func = func;
    trav.key = &key;
    trav_ref(trav, parent, ref, mode, cb);
}

void trav_elem (
    const Traversal& parent, const Accessor* acr, usize index,
    AccessMode mode, TravCallback cb
) {
    assert(&parent == current_traversal);
    Traversal trav;
    trav.op = ELEM;
    trav.index = index;
    trav_acr(trav, parent, acr, mode, cb);
}

void trav_elem_func (
    const Traversal& parent, const Reference& ref,
    Reference(* func )(Mu&, usize), usize index, AccessMode mode, TravCallback cb
) {
    assert(&parent == current_traversal);
    Traversal trav;
    trav.op = ELEM_FUNC;
    trav.elem_func = func;
    trav.index = index;
    trav_ref(trav, parent, ref, mode, cb);
}

Reference trav_reference (const Traversal& trav) {
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
            default: AYU_INTERNAL_UGUU();
        }
    }
}

Location trav_location (const Traversal& trav) {
    if (trav.op == START) {
        return *trav.location;
    }
    else {
        Location parent = trav_location(*trav.parent);
        switch (trav.op) {
            case DELEGATE: return parent;
            case ATTR: case ATTR_FUNC:
                return Location(parent, *trav.key);
            case ELEM: case ELEM_FUNC:
                return Location(parent, trav.index);
            default: AYU_INTERNAL_UGUU();
        }
    }
}

} // namespace ayu::in
