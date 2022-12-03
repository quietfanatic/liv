#include "traversal-private.h"

#include "../reference.h"

namespace ayu::in {

void trav_start (
    const Reference& ref, const Location& loc, AccessOp op, TravCallback cb
) {
    Traversal trav;
    trav.parent = null;
    trav.desc = DescriptionPrivate::get(ref.type());
    trav.readonly = ref.readonly();
    trav.type = START;
    trav.reference = &ref;
    trav.location = &loc;
    if (Mu* address = ref.address()) {
        trav.item = address;
        trav.addressable = true;
        cb(trav);
    }
    else {
        ref.access(op, [&](Mu& v){
            trav.item = &v;
            trav.addressable = false;
            cb(trav);
        });
    }
}

void trav_delegate (
    const Traversal& parent, const Accessor* acr, AccessOp op, TravCallback cb
) {
    Traversal trav;
    trav.parent = &parent;
    trav.desc = DescriptionPrivate::get(acr->type(parent.item));
    trav.readonly = parent.readonly || acr->accessor_flags & ACR_READONLY;
    trav.type = DELEGATE;
    trav.acr = acr;
    if (Mu* address = acr->address(*parent.item)) {
        trav.item = address;
        trav.addressable = parent.addressable;
        cb(trav);
    }
    else {
        acr->access(op, *parent.item, [&](Mu& v){
            trav.item = &v;
            trav.addressable = false;
            cb(trav);
        });
    }
}

void trav_attr (
    const Traversal& parent, const Accessor* acr, const Str& key,
    AccessOp op, TravCallback cb
) {
    Traversal trav;
    trav.parent = &parent;
    trav.desc = DescriptionPrivate::get(acr->type(parent.item));
    trav.readonly = parent.readonly || acr->accessor_flags & ACR_READONLY;
    trav.type = ATTR;
    trav.acr = acr;
    trav.key = &key;
    if (Mu* address = acr->address(*parent.item)) {
        trav.item = address;
        trav.addressable = parent.addressable;
        cb(trav);
    }
    else {
        acr->access(op, *parent.item, [&](Mu& v){
            trav.item = &v;
            trav.addressable = false;
            cb(trav);
        });
    }
}

void trav_attr_func (
    const Traversal& parent, Reference ref, Reference(* func )(Mu&, Str),
    const Str& key, AccessOp op, TravCallback cb
) {
    Traversal trav;
    trav.parent = &parent;
    trav.desc = DescriptionPrivate::get(ref.type());
    trav.readonly = parent.readonly || ref.readonly();
    trav.type = ATTR_FUNC;
    trav.attr_func = func;
    trav.key = &key;
    if (Mu* address = ref.address()) {
        trav.item = address;
        trav.addressable = parent.addressable;
        cb(trav);
    }
    else {
        ref.access(op, [&](Mu& v){
            trav.item = &v;
            trav.addressable = false;
            cb(trav);
        });
    }
}

void trav_elem (
    const Traversal& parent, const Accessor* acr, usize index,
    AccessOp op, TravCallback cb
) {
    Traversal trav;
    trav.parent = &parent;
    trav.desc = DescriptionPrivate::get(acr->type(parent.item));
    trav.readonly = parent.readonly || acr->accessor_flags & ACR_READONLY;
    trav.type = ELEM;
    trav.acr = acr;
    trav.index = index;
    if (Mu* address = acr->address(*parent.item)) {
        trav.item = address;
        trav.addressable = parent.addressable;
        cb(trav);
    }
    else {
        acr->access(op, *parent.item, [&](Mu& v){
            trav.item = &v;
            trav.addressable = false;
            cb(trav);
        });
    }
}

void trav_elem_func (
    const Traversal& parent, Reference ref, Reference(* func )(Mu&, usize),
    usize index, AccessOp op, TravCallback cb
) {
    Traversal trav;
    trav.parent = &parent;
    trav.desc = DescriptionPrivate::get(ref.type());
    trav.readonly = parent.readonly || ref.readonly();
    trav.type = ELEM_FUNC;
    trav.elem_func = func;
    trav.index = index;
    if (Mu* address = ref.address()) {
        trav.item = address;
        trav.addressable = parent.addressable;
        cb(trav);
    }
    else {
        ref.access(op, [&](Mu& v){
            trav.item = &v;
            trav.addressable = false;
            cb(trav);
        });
    }
}

Reference trav_reference (const Traversal& trav) {
    if (trav.addressable) {
        if (trav.readonly) {
            return Reference(trav.item, &trav.desc->readonly_identity_acr);
        }
        else {
            return Reference(trav.item, &trav.desc->identity_acr);
        }
    }
    else if (trav.type == START) {
        return *trav.reference;
    }
    else {
        Reference parent = trav_reference(*trav.parent);
        switch (trav.type) {
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
    if (trav.type == START) {
        return *trav.location;
    }
    else {
        Location parent = trav_location(*trav.parent);
        switch (trav.type) {
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
