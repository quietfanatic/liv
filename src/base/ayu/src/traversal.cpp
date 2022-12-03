#include "traversal-private.h"

#include "../reference.h"

namespace ayu::in {

struct StartTraversal : Traversal {
    const Reference* reference;
    const Location* location;
};
struct DelegateTraversal : Traversal {
    const Accessor* acr;
};
struct AttrTraversal : Traversal {
    const Accessor* acr;
    Str key;
};
struct AttrFuncTraversal : Traversal {
    Reference(* attr_func )(Mu&, Str);
    Str key;
};
struct ElemTraversal : Traversal {
    const Accessor* acr;
    usize index;
};
struct ElemFuncTraversal : Traversal {
    Reference(* elem_func )(Mu&, usize);
    usize index;
};

void trav_start (
    const Reference& ref, const Location& loc, AccessOp op, TravCallback cb
) {
    StartTraversal trav;
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
    DelegateTraversal trav;
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
    const Traversal& parent, const Accessor* acr, Str key,
    AccessOp op, TravCallback cb
) {
    AttrTraversal trav;
    trav.parent = &parent;
    trav.desc = DescriptionPrivate::get(acr->type(parent.item));
    trav.readonly = parent.readonly || acr->accessor_flags & ACR_READONLY;
    trav.type = ATTR;
    trav.acr = acr;
    trav.key = key;
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
    Str key, AccessOp op, TravCallback cb
) {
    AttrFuncTraversal trav;
    trav.parent = &parent;
    trav.desc = DescriptionPrivate::get(ref.type());
    trav.readonly = parent.readonly || ref.readonly();
    trav.type = ATTR_FUNC;
    trav.attr_func = func;
    trav.key = key;
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
    ElemTraversal trav;
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
    ElemFuncTraversal trav;
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
    else switch (trav.type) {
        case START: {
            return *trav.as<StartTraversal>().reference;
        }
        case DELEGATE: {
            auto& self = trav.as<DelegateTraversal>();
            return trav_reference(*self.parent).chain(self.acr);
        }
        case ATTR: {
            auto& self = trav.as<AttrTraversal>();
            return trav_reference(*self.parent).chain(self.acr);
        }
        case ATTR_FUNC: {
            auto& self = trav.as<AttrFuncTraversal>();
            return trav_reference(*self.parent).chain_attr_func(
                self.attr_func, self.key
            );
        }
        case ELEM: {
            auto& self = trav.as<ElemTraversal>();
            return trav_reference(*self.parent).chain(self.acr);
        }
        case ELEM_FUNC: {
            auto& self = trav.as<ElemFuncTraversal>();
            return trav_reference(*self.parent).chain_elem_func(
                self.elem_func, self.index
            );
        }
        default: AYU_INTERNAL_UGUU();
    }
}

Location trav_location (const Traversal& trav) {
    switch (trav.type) {
        case START: {
            return *trav.as<StartTraversal>().location;
        }
        case DELEGATE: {
            auto& self = trav.as<DelegateTraversal>();
            return trav_location(*self.parent);
        }
        case ATTR: {
            auto& self = trav.as<AttrTraversal>();
            return Location(trav_location(*self.parent), self.key);
        }
        case ATTR_FUNC: {
            auto& self = trav.as<AttrFuncTraversal>();
            return Location(trav_location(*self.parent), self.key);
        }
        case ELEM: {
             // TODO: this is INCORRECT because it ignores inherited elems.
             // Inheritance of elems can't be ignored, unlike attrs.
            auto& self = trav.as<ElemTraversal>();
            return Location(trav_location(*self.parent), self.index);
        }
        case ELEM_FUNC: {
            auto& self = trav.as<ElemFuncTraversal>();
            return Location(trav_location(*self.parent), self.index);
        }
        default: AYU_INTERNAL_UGUU();
    }
}

} // namespace ayu::in
