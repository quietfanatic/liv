#include "traversal-private.h"

#include "../reference.h"

namespace ayu::in {

void trav_start (
    const Reference& ref, const Location& loc, AccessMode mode, TravCallback cb
) {
    Traversal trav;
    trav.parent = null;
    trav.desc = DescriptionPrivate::get(ref.type());
    trav.readonly = ref.readonly();
    trav.only_addressable = false;
    trav.op = START;
    trav.reference = &ref;
    trav.location = &loc;
    if (Mu* address = ref.address()) {
        trav.item = address;
        trav.addressable = true;
        cb(trav);
    }
    else {
        ref.access(mode, [&](Mu& v){
            trav.item = &v;
            trav.addressable = false;
            cb(trav);
        });
    }
}

void trav_start_addressable (
    Pointer ptr, const Location& loc, TravCallback cb
) {
    Traversal trav;
    trav.parent = null;
    trav.desc = DescriptionPrivate::get(ptr.type);
    trav.readonly = false;
    trav.only_addressable = true;
    trav.op = START;
     // Unneeded
    // trav.reference = Reference(type, addr)
    trav.location = &loc;
    trav.item = ptr.address;
    trav.addressable = true;
    cb(trav);
}

static void trav_acr (
    Traversal& trav, const Traversal& parent, const Accessor* acr,
    AccessMode mode, TravCallback cb
) {
    trav.parent = &parent;
    trav.desc = DescriptionPrivate::get(acr->type(parent.item));
    trav.readonly = parent.readonly || acr->accessor_flags & ACR_READONLY;
    trav.only_addressable = parent.only_addressable;
    trav.acr = acr;
    if (Mu* address = acr->address(*parent.item)) {
        trav.item = address;
        trav.addressable = parent.addressable;
        cb(trav);
    }
    else if (!trav.only_addressable) {
        acr->access(mode, *parent.item, [&](Mu& v){
            trav.item = &v;
            trav.addressable = false;
            cb(trav);
        });
    }
}

static void trav_ref (
    Traversal& trav, const Traversal& parent, const Reference& ref,
    AccessMode mode, TravCallback cb
) {
    trav.parent = &parent;
    trav.desc = DescriptionPrivate::get(ref.type());
    trav.readonly = parent.readonly || ref.readonly();
    trav.only_addressable = parent.only_addressable;
    if (Mu* address = ref.address()) {
        trav.item = address;
        trav.addressable = parent.addressable;
        cb(trav);
    }
    else if (!trav.only_addressable) {
        ref.access(mode, [&](Mu& v){
            trav.item = &v;
            trav.addressable = false;
            cb(trav);
        });
    }
}

void trav_delegate (
    const Traversal& parent, const Accessor* acr, AccessMode mode, TravCallback cb
) {
    Traversal trav;
    trav.op = DELEGATE;
    trav_acr(trav, parent, acr, mode, cb);
}

void trav_attr (
    const Traversal& parent, const Accessor* acr, const Str& key,
    AccessMode mode, TravCallback cb
) {
    Traversal trav;
    trav.op = ATTR;
    trav.key = &key;
    trav_acr(trav, parent, acr, mode, cb);
}

void trav_attr_func (
    const Traversal& parent, const Reference& ref,
    Reference(* func )(Mu&, Str), const Str& key, AccessMode mode, TravCallback cb
) {
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
    Traversal trav;
    trav.op = ELEM;
    trav.index = index;
    trav_acr(trav, parent, acr, mode, cb);
}

void trav_elem_func (
    const Traversal& parent, const Reference& ref,
    Reference(* func )(Mu&, usize), usize index, AccessMode mode, TravCallback cb
) {
    Traversal trav;
    trav.op = ELEM_FUNC;
    trav.elem_func = func;
    trav.index = index;
    trav_ref(trav, parent, ref, mode, cb);
}

Reference trav_reference (const Traversal& trav) {
    if (trav.addressable) {
        return Pointer(trav.item,
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
