#pragma once

#include "../internal/descriptors-internal.h"

namespace ayu::in {

template <class C>
const C* offset_get (const void* base, uint16 offset) {
    if (!offset) return null;
    return (const C*)((char*)base + offset);
}

struct ValueDcrPrivate : ValueDcr<Mu> {
    Mu* get_value () const {
        if (address) return (Mu*)address;
        else return (Mu*)((char*)this + sizeof(ValueDcr<Mu>));
    }
};

struct ValuesDcrPrivate : ValuesDcr<Mu> {
    const ValueDcrPrivate* value (uint16 i) const {
        return offset_get<ValueDcrPrivate>(this, (&n_values)[i+1]);
    }
};

struct AttrDcrPrivate : AttrDcr<Mu> {
    const Accessor* acr () const {
         // We have to take a somewhat roundabout way to get a pointer to acr,
         // because we can't instantiate AttrDcrWith<Mu, Accessor> because
         // Accessor is abstract, and we can't pretend it's a concrete Accessor
         // type because then the optimizer will devirtualize method calls
         // incorrectly.
         //
         // The Accessor should be right after the attr base in memory, without
         // any padding.  This should be the case if vtable pointers have the
         // same alignment as StaticString and there's nothing else funny going
         // on.
         //
         // TODO: We may be able to simplify this now that we're using our own
         // vtables.
        static_assert(sizeof(AttrDcr<Mu>) % alignof(Accessor) == 0);
        return (const Accessor*)((char*)this + sizeof(AttrDcr<Mu>));
    }
};

struct AttrsDcrPrivate : AttrsDcr<Mu> {
    const AttrDcrPrivate* attr (uint16 i) const {
        return offset_get<AttrDcrPrivate>(this, (&n_attrs)[i+1]);
    }
};

struct ElemDcrPrivate : ElemDcr<Mu> {
    const Accessor* acr () const {
         // This is much easier than attrs...as long as we don't add
         // anything to ElemDcr...
        return (const Accessor*)this;
    }
};

struct ElemsDcrPrivate : ElemsDcr<Mu> {
    const ElemDcrPrivate* elem (uint16 i) const {
        return offset_get<ElemDcrPrivate>(this, (&n_elems)[i+1]);
    }
};

struct DescriptionPrivate : DescriptionFor<Mu> {
    static const DescriptionPrivate* get (Type t) {
        return reinterpret_cast<const DescriptionPrivate*>(t.data & ~1);
    }
    const ToTreeDcr<Mu>* to_tree () const {
        return offset_get<ToTreeDcr<Mu>>(this, to_tree_offset);
    }
    const FromTreeDcr<Mu>* from_tree () const {
        return offset_get<FromTreeDcr<Mu>>(this, from_tree_offset);
    }
    const SwizzleDcr<Mu>* swizzle () const {
        return offset_get<SwizzleDcr<Mu>>(this, swizzle_offset);
    }
    const InitDcr<Mu>* init () const {
        return offset_get<InitDcr<Mu>>(this, init_offset);
    }
    const ValuesDcrPrivate* values () const {
        return offset_get<ValuesDcrPrivate>(this, values_offset);
    }
    const AttrsDcrPrivate* attrs () const {
        return offset_get<AttrsDcrPrivate>(this, attrs_offset);
    }
    const ElemsDcrPrivate* elems () const {
        return offset_get<ElemsDcrPrivate>(this, elems_offset);
    }
    const Accessor* keys_acr () const {
        return offset_get<Accessor>(this, keys_offset);
    }
    const AttrFuncDcr<Mu>* attr_func () const {
        return offset_get<AttrFuncDcr<Mu>>(this, attr_func_offset);
    }
    const Accessor* length_acr () const {
        return offset_get<Accessor>(this, length_offset);
    }
    const ElemFuncDcr<Mu>* elem_func () const {
        return offset_get<ElemFuncDcr<Mu>>(this, elem_func_offset);
    }
    const Accessor* delegate_acr () const {
        return offset_get<Accessor>(this, delegate_offset);
    }

    bool accepts_object () const {
        return attrs_offset || keys_offset;
    }
    bool accepts_array () const {
        return elems_offset || length_offset;
    }
     // Figure out whether this description prefers being serialized as an array
     // or as an object.  Whichever has a related facet specified first will
     // be picked.
    uint16 preference () const {
         // We've bumped this calculation up to compile-time.
        return flags & PREFERENCE;
    }
};

} // namespace ayu::in
