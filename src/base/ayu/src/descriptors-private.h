#pragma once

#include "../internal/descriptors-internal.h"

#include "tree-private.h"

namespace ayu::in {

template <class C>
const C* offset_get (const void* base, uint16 offset) {
    if (!offset) return null;
    return (const C*)((char*)base + offset);
}

struct ValueDcrPrivate : ValueDcr<Mu> {
    const void* name () const {
        switch (form) {
            case VFNULL: return &((const ValueDcrWith<void*, Null, false>*)this)->name;
            case VFBOOL: return &((const ValueDcrWith<void*, bool, false>*)this)->name;
            case VFINT64: return &((const ValueDcrWith<void*, int64, false>*)this)->name;
            case VFDOUBLE: return &((const ValueDcrWith<void*, double, false>*)this)->name;
            case VFSTR: return &((const ValueDcrWith<void*, Str, false>*)this)->name;
            default: AYU_INTERNAL_UGUU();
        }
    }
    Mu* get_value () const {
         // This looks awful but it should be pretty optimizable...
        if (pointer) switch (form) {
            case VFNULL: return (Mu*)((const ValueDcrWith<Mu, Null, true>*)this)->ptr;
            case VFBOOL: return (Mu*)((const ValueDcrWith<Mu, bool, true>*)this)->ptr;
            case VFINT64: return (Mu*)((const ValueDcrWith<Mu, int64, true>*)this)->ptr;
            case VFDOUBLE: return (Mu*)((const ValueDcrWith<Mu, double, true>*)this)->ptr;
            case VFSTR: return (Mu*)((const ValueDcrWith<Mu, Str, true>*)this)->ptr;
            default: AYU_INTERNAL_UGUU();
        }
        else switch (form) {
            case VFNULL: return (Mu*)&((const ValueDcrWith<void*, Null, false>*)this)->value;
            case VFBOOL: return (Mu*)&((const ValueDcrWith<void*, bool, false>*)this)->value;
            case VFINT64: return (Mu*)&((const ValueDcrWith<void*, int64, false>*)this)->value;
            case VFDOUBLE: return (Mu*)&((const ValueDcrWith<void*, double, false>*)this)->value;
            case VFSTR: return (Mu*)&((const ValueDcrWith<void*, Str, false>*)this)->value;
            default: AYU_INTERNAL_UGUU();
        }
    }
    Tree value_to_tree (
        const ValuesDcr<Mu>* values, Mu& v, TreeFlags flags
    ) const {
        if (values->compare(v, *get_value())) {
            switch (form) {
                case VFNULL: return Tree(null, flags);
                case VFBOOL: return Tree(*(const bool*)name(), flags);
                case VFINT64: return Tree(*(const int64*)name(), flags);
                case VFDOUBLE: return Tree(*(const double*)name(), flags);
                case VFSTR: return Tree(*(Str*)name(), flags);
                default: AYU_INTERNAL_UGUU();
            }
        }
        else return Tree();
    }
    bool matches_tree (Tree tree) const {
        switch (form) {
            case VFNULL:
                return tree.form() == NULLFORM;
            case VFBOOL:
                return tree.form() == BOOL
                    && tree.data->as_known<bool>() == *(const bool*)name();
            case VFINT64:
                return tree.form() == NUMBER
                    && tree == Tree(*(const int64*)name());
            case VFDOUBLE:
                if (tree.form() == NUMBER) {
                    double a = double(tree);
                    double b = *(const double*)name();
                    return a == b || (a != a && b != b);
                }
                else return false;
            case VFSTR:
                return tree.form() == STRING
                    && tree.data->as_known<String>() == *(const Str*)name();
            default: AYU_INTERNAL_UGUU();
        }
    }
    Mu* tree_to_value (Tree tree) const {
        if (matches_tree(tree)) return get_value();
        else return null;
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
         // same alignment as Str and there's nothing else funny going on.
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
