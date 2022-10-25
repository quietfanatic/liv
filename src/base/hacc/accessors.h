// This module contains the classes implementing the accessors that can be used
// in haccable descriptions.

#pragma once

// TODO: Figure out how to get around strict-aliasing rules

#include <cassert>
#include <typeinfo>

#include "common.h"
#include "type.h"

namespace hacc::X {
    struct WriteReadonlyAccessor : LogicError { };
}

namespace hacc::in {

///// UNIVERSAL ACCESSOR STUFF

enum AccessorFlags {
     // Writes through this accessor will fail.  Attrs and elems with this
     //  accessor will not be serialized.
    ACR_READONLY = 0x1,
     // Normally address() is only usable if all links in an accessor chain
     //  are addressable.  However, if this is set, this accessor's address()
     //  is usable even if the accessor above it is not addressable.  This
     //  allows for reference-like objects to be accessed through value_funcs
     //  or similar, but have their derived references still be addressable.
    ACR_ANCHORED_TO_PARENT = 0x2
};
static constexpr AccessorFlags operator | (const AccessorFlags& a, const AccessorFlags& b) {
    return AccessorFlags(int(a)|int(b));
}

 // These belong on AttrDcr and ElemDcr, but we're putting them with the
 //  accessor flags to save space.
enum AttrFlags {
     // If this is set, the attr doesn't need to be present when doing
     //  the from_tree operation.  There's no support for default values here;
     //  if an attr wants a default value, set it in the class's default
     //  constructor.  This is allowed on elems, but all optional elems must
     //  follow all non-optional elems (allowing optional elems in the middle
     //  would change the apparent index of later required elems, which would
     //  be confusing).
    ATTR_OPTIONAL = 0x1,
     // If this is set, the attrs of this attr will be included in the
     //  serialization of this item and available through calls to attr().  In
     //  addition, this item will be able to be upcasted to the type of the attr
     //  if it is addressable.  This is not currently supported on elems.
    ATTR_INHERIT = 0x2
};
static constexpr AttrFlags operator | (const AttrFlags& a, const AttrFlags& b) {
    return AttrFlags(int(a)|int(b));
}

enum AccessOp {
     // Provides a const ref containing the value of the object.  It may refer
     //  to the object itself or to a temporary that will go out of scope when
     //  read() returns.
    ACR_READ = 1,
     // Provides a ref to which a new value can be written.  It may refer to the
     //  object itself, or it may be a reference to a default-constructed
     //  temporary.  Neglecting to write to the reference in the callback may
     //  clear the object.
    ACR_WRITE = 2,
     // Provides a ref containing the value of the object, to which a new value
     //  can be written.  May be implemented as a read followed by a write.
    ACR_MODIFY = 3
};

struct Accessor;

 // Rolling our own vtables because the interaction between builtin vtables and
 //  template classes creates more overhead than we would like.
struct AccessorVT {
    static Mu* default_address (const Accessor*, Mu&) { return null; }
    Type(* type )(const Accessor*, const Mu&) = null;
    void(* access )(const Accessor*, AccessOp, Mu&, Callback<void(Mu&)>) = null;
    Mu*(* address )(const Accessor*, Mu&) = &default_address;
    void(* destroy )(Accessor*) = [](Accessor*){ };
};

struct Accessor {
    static constexpr AccessorVT _vt = {};
    const AccessorVT* vt = &_vt;
     // If this is uint16(-1), it's a constexpr accessor and shouldn't be
     //  modified.  Yes, this does mean that if an accessor accumulates 65535
     //  references it won't be deleted.  I doubt you'll care.
     // (Can't have mutable members in constexpr objects)
    uint16 ref_count = 0;
    uint8 accessor_flags = 0;
     // These belong on AttrDcr and ElemDcr but we're storing them here to
     //  save space.
    uint8 attr_flags = 0;

    explicit constexpr Accessor (const AccessorVT* vt, uint8 flags = 0) :
        vt(vt), accessor_flags(flags)
    { }

    Type type (const Mu& from) const { return vt->type(this, from); }
    void access (AccessOp op, Mu& from, Callback<void(Mu&)> cb) const {
        if (op != ACR_READ && accessor_flags & ACR_READONLY) {
            throw X::WriteReadonlyAccessor();
        }
        vt->access(this, op, from, cb);
    }
    void read (const Mu& from, Callback<void(const Mu&)> cb) const {
        access(ACR_READ, const_cast<Mu&>(from), reinterpret_cast<Callback<void(Mu&)>&>(cb));
    }
    void write (Mu& from, Callback<void(Mu&)> cb) const {
        access(ACR_WRITE, from, cb);
    }
    void modify (Mu& from, Callback<void(Mu&)> cb) const {
        access(ACR_MODIFY, from, cb);
    }
    Mu* address (Mu& from) const { return vt->address(this, from); }

    void inc () const {
        if (ref_count != uint16(-1)) {
            const_cast<uint16&>(ref_count)++;
        }
    }
    void dec () const {
        if (ref_count != uint16(-1)) {
            if (!--const_cast<uint16&>(ref_count)) {
                vt->destroy(const_cast<Accessor*>(this));
                delete this;
            }
        }
    }
};

template <class Acr>
static constexpr Acr constexpr_acr (const Acr& a) {
    Acr r = a;
    r.ref_count = uint16(-1);
    return r;
}

 // A struct representing a pointer to a refcounted accessor, with an
 //  optimization for simple pass-through with type.
struct AccessorOrType {
    enum AccessorForm {
        CE_ACR = 0,
        ACR = 1,
        TYPE = 2,
        TYPE_READONLY = 3,
    };
    usize data;
    uint8 form () const { return data & 3; }
    Accessor* as_acr () const {
        return reinterpret_cast<Accessor*>(data & ~usize(3));
    }
    Type as_type () const {
        return reinterpret_cast<const in::Description*>(data & ~usize(3));
    }

    constexpr AccessorOrType (Null) : data(0) { }
    AccessorOrType (const Accessor* acr) :
        data(reinterpret_cast<usize>(acr))
    {
        if (!acr || reinterpret_cast<usize>(acr) & 3) {
            HACC_INTERNAL_ERROR();
        }
        if (acr && acr->ref_count != uint16(-1)) {
            data |= ACR;
            acr->inc();
        }
    }
    AccessorOrType (Type t, bool readonly = false) :
        data(reinterpret_cast<usize>(t.desc) | (readonly ? TYPE_READONLY : TYPE))
    {
        if (!t || reinterpret_cast<usize>(t.desc) & 3) {
            HACC_INTERNAL_ERROR();
        }
    }
    AccessorOrType (const AccessorOrType& o) : data(o.data) {
        if (form() == ACR) { as_acr()->inc(); }
    }
    AccessorOrType (AccessorOrType&& o) : data(o.data) { o.data = 0; }
    ~AccessorOrType () {
        if (form() == ACR) { as_acr()->dec(); }
    }

    AccessorOrType& operator = (const AccessorOrType& o) {
        this->~AccessorOrType();
        new (this) AccessorOrType(o);
        return *this;
    }
    AccessorOrType& operator = (AccessorOrType&& o) {
        this->~AccessorOrType();
        new (this) AccessorOrType(std::move(o));
        return *this;
    }

    explicit operator bool () const { return data; }

    bool readonly () const {
        switch (form()) {
            case CE_ACR:
            case ACR: return as_acr()->accessor_flags & ACR_READONLY;
            case TYPE: return false;
            case TYPE_READONLY: return true;
            default: HACC_INTERNAL_ERROR();
        }
    }
    Type type (const Mu& from) const {
        switch (form()) {
            case CE_ACR:
            case ACR: return as_acr()->type(from);
            case TYPE:
            case TYPE_READONLY: return as_type();
            default: HACC_INTERNAL_ERROR();
        }
    }
    void access (AccessOp op, Mu& from, Callback<void(Mu&)> cb) const {
        switch (form()) {
            case CE_ACR:
            case ACR: return as_acr()->access(op, from, cb);
            case TYPE: return cb(from);
            case TYPE_READONLY: {
                if (op != ACR_READ) throw X::WriteReadonlyAccessor();
                return cb(from);
            }
            default: HACC_INTERNAL_ERROR();
        }
    }
    Mu* address (Mu& from) const {
        switch (form()) {
            case CE_ACR:
            case ACR: return as_acr()->address(from);
            case TYPE:
            case TYPE_READONLY: return &from;
            default: HACC_INTERNAL_ERROR();
        }
    }
};
static bool operator == (const AccessorOrType& a, const AccessorOrType& b) {
    return a.data == b.data;
}
static bool operator != (const AccessorOrType& a, const AccessorOrType& b) {
    return a.data != b.data;
}

///// ACCESSOR TYPES

/// base

 // TODO: Detect when From and To have the same address and erase both types.
template <class From, class To>
struct BaseAcr2 : Accessor {
    using AccessorFromType = From;
    using AccessorToType = To;
    static Type _type (const Accessor*, const Mu&) {
        return Type::CppType<To>();
    }
    static void _access (const Accessor*, AccessOp, Mu& from, Callback<void(Mu&)> cb) {
        To& to = reinterpret_cast<From&>(from);
        cb(reinterpret_cast<Mu&>(to));
    }
    static Mu* _address (const Accessor*, Mu& from) {
        To& to = reinterpret_cast<From&>(from);
        return &reinterpret_cast<Mu&>(to);
    }
    static constexpr AccessorVT _vt = {&_type, &_access, &_address};
    explicit constexpr BaseAcr2 (uint8 flags = 0) : Accessor(&_vt, flags) { }
};

/// member

struct MemberAcr0 : Accessor {
    static Type _type (const Accessor*, const Mu&);
    static void _access (const Accessor*, AccessOp, Mu&, Callback<void(Mu&)>);
    static Mu* _address (const Accessor*, Mu&);
     // TODO: move to .cpp?
    static constexpr AccessorVT _vt = {&_type, &_access, &_address};
    using Accessor::Accessor;
};
template <class From, class To>
struct MemberAcr2 : MemberAcr0 {
    using AccessorFromType = From;
    using AccessorToType = To;
    Type(* get_type )();
    To From::* mp;
    explicit constexpr MemberAcr2 (To From::* mp, uint8 flags = 0) :
        MemberAcr0(&_vt, flags), get_type(&Type::CppType<To>), mp(mp)
    { }
};

/// ref_func

struct RefFuncAcr0 : Accessor {
     // It's the programmer's responsibility to know whether they're
     //  allowed to address this reference or not.
    static Type _type (const Accessor*, const Mu&);
    static void _access (const Accessor*, AccessOp, Mu&, Callback<void(Mu&)>);
    static Mu* _address (const Accessor*, Mu&);
    static constexpr AccessorVT _vt = {&_type, &_access, &_address};
    using Accessor::Accessor;
};
template <class From, class To>
struct RefFuncAcr2 : RefFuncAcr0 {
    using AccessorFromType = From;
    using AccessorToType = To;
    Type(* get_type )();
    To&(* f )(From&);
    explicit constexpr RefFuncAcr2 (To&(* f )(From&), uint8 flags = 0) :
        RefFuncAcr0(&_vt, flags), get_type(&Type::CppType<To>), f(f)
    { }
};

/// const_ref_func

struct ConstRefFuncAcr0 : Accessor {
     // It's the programmer's responsibility to know whether they're
     //  allowed to address this reference or not.
    static Type _type (const Accessor*, const Mu&);
    static void _access (const Accessor*, AccessOp, Mu&, Callback<void(Mu&)>);
    static Mu* _address (const Accessor*, Mu&);
    static constexpr AccessorVT _vt = {&_type, &_access, &_address};
    using Accessor::Accessor;
};
template <class From, class To>
struct ConstRefFuncAcr2 : ConstRefFuncAcr0 {
    using AccessorFromType = From;
    using AccessorToType = To;
    Type(* get_type )();
    const To&(* f )(const From&);
    explicit constexpr ConstRefFuncAcr2 (const To&(* f )(const From&), uint8 flags = 0) :
        ConstRefFuncAcr0(&_vt, flags), get_type(&Type::CppType<To>()), f(f)
    { }
};

/// const_ref_funcs

template <class To>
struct RefFuncsAcr1 : Accessor {
    static Type _type (const Accessor*, const Mu&);
    static void _access (const Accessor*, AccessOp, Mu&, Callback<void(Mu&)>);
    static constexpr AccessorVT _vt = {&_type, &_access};
    using Accessor::Accessor;
};
template <class From, class To>
struct RefFuncsAcr2 : RefFuncsAcr1<To> {
    using AccessorFromType = From;
    using AccessorToType = To;
    const To&(* getter )(const From&);
    void(* setter )(From&, const To&);
    explicit constexpr RefFuncsAcr2 (
        const To&(* g )(const From&),
        void(* s )(From&, const To&),
        uint8 flags = 0
    ) :
        RefFuncsAcr1<To>(&RefFuncsAcr1<To>::_vt, flags), getter(g), setter(s)
    { }
};
template <class To>
Type RefFuncsAcr1<To>::_type (const Accessor*, const Mu&) { return Type::CppType<To>(); }
template <class To>
void RefFuncsAcr1<To>::_access (const Accessor* acr, AccessOp op, Mu& from, Callback<void(Mu&)> cb_mu) {
    auto self = static_cast<const RefFuncsAcr2<Mu, To>*>(acr);
    auto& cb = reinterpret_cast<Callback<void(To&)>&>(cb_mu);
    switch (op) {
        case ACR_READ: {
            return reinterpret_cast<Callback<void(const To&)>&>(cb)(self->getter(from));
        }
        case ACR_WRITE: {
            To tmp;
            cb(tmp);
            return self->setter(from, tmp);
        }
        case ACR_MODIFY: {
            To tmp = self->getter(from);
            cb(tmp);
            return self->setter(from, tmp);
        }
    }
}

///// value_func

template <class To>
struct ValueFuncAcr1 : Accessor {
    static Type _type (const Accessor*, const Mu&);
    static void _access (const Accessor*, AccessOp, Mu&, Callback<void(Mu&)>);
    static constexpr AccessorVT _vt = {&_type, &_access};
    using Accessor::Accessor;
};
template <class From, class To>
struct ValueFuncAcr2 : ValueFuncAcr1<To> {
    using AccessorFromType = From;
    using AccessorToType = To;
    To(* f )(const From&);
    explicit constexpr ValueFuncAcr2 (To(* f )(const From&), uint8 flags = 0) :
        ValueFuncAcr1<To>(&ValueFuncAcr1<To>::_vt, flags | ACR_READONLY), f(f)
    { }
};
template <class To>
Type ValueFuncAcr1<To>::_type (const Accessor*, const Mu&) { return Type::CppType<To>(); }
template <class To>
void ValueFuncAcr1<To>::_access (const Accessor* acr, AccessOp op, Mu& from, Callback<void(Mu&)> cb) {
    if (op != ACR_READ) throw X::WriteReadonlyAccessor();
    auto self = static_cast<const ValueFuncAcr2<Mu, To>*>(acr);
    reinterpret_cast<Callback<void(const To&)>&>(cb)(self->f(from));
}

/// value_funcs

template <class To>
struct ValueFuncsAcr1 : Accessor {
    static Type _type (const Accessor*, const Mu&);
    static void _access (const Accessor*, AccessOp, Mu&, Callback<void(Mu&)>);
    static constexpr AccessorVT _vt = {&_type, &_access};
    using Accessor::Accessor;
};
template <class From, class To>
struct ValueFuncsAcr2 : ValueFuncsAcr1<To> {
    using AccessorFromType = From;
    using AccessorToType = To;
    To(* getter )(const From&);
    void(* setter )(From&, To);
    explicit constexpr ValueFuncsAcr2 (
        To(* g )(const From&),
        void(* s )(From&, To),
        uint8 flags = 0
    ) :
        ValueFuncsAcr1<To>(&ValueFuncsAcr1<To>::_vt, flags), getter(g), setter(s)
    { }
};
template <class To>
Type ValueFuncsAcr1<To>::_type (const Accessor*, const Mu&) { return Type::CppType<To>(); }
template <class To>
void ValueFuncsAcr1<To>::_access (const Accessor* acr, AccessOp op, Mu& from, Callback<void(Mu&)> cb_mu) {
    auto self = static_cast<const ValueFuncsAcr2<Mu, To>*>(acr);
    auto& cb = reinterpret_cast<Callback<void(To&)>&>(cb_mu);
    switch (op) {
        case ACR_READ: {
            return reinterpret_cast<Callback<void(const To&)>&>(cb)(self->getter(from));
        }
        case ACR_WRITE: {
            To tmp;
            cb(tmp);
            return self->setter(from, std::move(tmp));
        }
        case ACR_MODIFY: {
            To tmp = self->getter(from);
            cb(tmp);
            return self->setter(from, std::move(tmp));
        }
    }
}

/// mixed_funcs

template <class To>
struct MixedFuncsAcr1 : Accessor {
    static Type _type (const Accessor*, const Mu&);
    static void _access (const Accessor*, AccessOp, Mu&, Callback<void(Mu&)>);
    static constexpr AccessorVT _vt = {&_type, &_access};
    using Accessor::Accessor;
};
template <class From, class To>
struct MixedFuncsAcr2 : MixedFuncsAcr1<To> {
    using AccessorFromType = From;
    using AccessorToType = To;
    To(* getter )(const From&);
    void(* setter )(From&, const To&);
    explicit constexpr MixedFuncsAcr2 (
        To(* g )(const From&),
        void(* s )(From&, const To&),
        uint8 flags = 0
    ) :
        MixedFuncsAcr1<To>(&MixedFuncsAcr1<To>::_vt, flags), getter(g), setter(s)
    { }
};
template <class To>
Type MixedFuncsAcr1<To>::_type (const Accessor*, const Mu&) { return Type::CppType<To>(); }
template <class To>
void MixedFuncsAcr1<To>::_access (const Accessor* acr, AccessOp op, Mu& from, Callback<void(Mu&)> cb_mu) {
    auto self = static_cast<const MixedFuncsAcr2<Mu, To>*>(acr);
    auto& cb = reinterpret_cast<Callback<void(To&)>&>(cb_mu);
    switch (op) {
        case ACR_READ: {
            return reinterpret_cast<Callback<void(const To&)>&>(cb)(self->getter(from));
        }
        case ACR_WRITE: {
            To tmp;
            cb(tmp);
            return self->setter(from, std::move(tmp));
        }
        case ACR_MODIFY: {
            To tmp = (self->getter)(from);
            cb(tmp);
            return self->setter(from, std::move(tmp));
        }
    }
}

/// assignable

template <class From, class To>
struct AssignableAcr2 : Accessor {
    using AccessorFromType = From;
    using AccessorToType = To;
    static Type _type (const Accessor*, const Mu&) { return Type::CppType<To>(); }
    static void _access (const Accessor*, AccessOp op, Mu& from_mu, Callback<void(Mu&)> cb_mu) {
        From& from = reinterpret_cast<From&>(from_mu);
        auto& cb = reinterpret_cast<Callback<void(To&)>&>(cb_mu);
        switch (op) {
            case ACR_READ: {
                To tmp;
                tmp = from;
                return cb(tmp);
            }
            case ACR_WRITE: {
                To tmp;
                cb(tmp);
                from = tmp;
                return;
            }
            case ACR_MODIFY: {
                To tmp;
                tmp = from;
                cb(tmp);
                from = tmp;
                return;
            }
        }
    }
    static constexpr AccessorVT _vt = {&_type, &_access};
    explicit constexpr AssignableAcr2 (uint8 flags = 0) : Accessor(&_vt, flags) { }
};

/// variable

template <class To>
struct VariableAcr1 : Accessor {
    static Type _type (const Accessor*, const Mu&);
    static void _access (const Accessor*, AccessOp, Mu&, Callback<void(Mu&)>);
     // This ACR cannot be addressable, because then Reference::chain and co.
     //  may take the address of value but then release this ACR object,
     //  invalidating value.
    static void _destroy (Accessor*);
    static constexpr AccessorVT _vt = {&_type, &_access, &AccessorVT::default_address, &_destroy};
    using Accessor::Accessor;
};
template <class From, class To>
struct VariableAcr2 : VariableAcr1<To> {
    using AccessorFromType = From;
    using AccessorToType = To;
    mutable To value;
     // This ACR cannot be constexpr.
    explicit VariableAcr2 (To&& v, uint8 flags = 0) :
        VariableAcr1<To>(&VariableAcr1<To>::_vt, flags), value(std::move(v))
    { }
};
template <class To>
Type VariableAcr1<To>::_type (const Accessor*, const Mu&) { return Type::CppType<To>(); }
template <class To>
void VariableAcr1<To>::_access (const Accessor* acr, AccessOp, Mu&, Callback<void(Mu&)> cb) {
    auto self = static_cast<const VariableAcr2<Mu, To>*>(acr);
    cb(reinterpret_cast<Mu&>(self->value));
}
template <class To>
void VariableAcr1<To>::_destroy (Accessor* acr) {
    auto self = static_cast<const VariableAcr2<Mu, To>*>(acr);
    self->~VariableAcr2<Mu, To>();
}

/// constant

template <class To>
struct ConstantAcr1 : Accessor {
    static Type _type (const Accessor*, const Mu&);
    static void _access (const Accessor*, AccessOp, Mu&, Callback<void(Mu&)>);
    static void _destroy (Accessor*);
    static constexpr AccessorVT _vt = {&_type, &_access, &AccessorVT::default_address, &_destroy};
    using Accessor::Accessor;
};
template <class From, class To>
struct ConstantAcr2 : ConstantAcr1<To> {
    using AccessorFromType = From;
    using AccessorToType = To;
    To value;
    explicit constexpr ConstantAcr2 (const To& v, uint8 flags = 0) :
        ConstantAcr1<To>(&ConstantAcr1<To>::_vt, flags | ACR_READONLY), value(v)
    { }
};
template <class To>
Type ConstantAcr1<To>::_type (const Accessor*, const Mu&) { return Type::CppType<To>(); }
template <class To>
void ConstantAcr1<To>::_access (const Accessor* acr, AccessOp op, Mu&, Callback<void(Mu&)> cb) {
    if (op != ACR_READ) throw X::WriteReadonlyAccessor();
    auto self = static_cast<const ConstantAcr2<Mu, To>*>(acr);
    cb(reinterpret_cast<Mu&>(const_cast<To&>(self->value)));
}
template <class To>
void ConstantAcr1<To>::_destroy (Accessor* acr) {
    auto self = static_cast<const ConstantAcr2<Mu, To>*>(acr);
    self->~ConstantAcr2<Mu, To>();
}

/// constant_pointer

struct ConstantPointerAcr0 : Accessor {
    static Type _type (const Accessor*, const Mu&);
    static void _access (const Accessor*, AccessOp, Mu&, Callback<void(Mu&)> cb);
     // Should be okay addressing this.
    static Mu* _address (const Accessor*, Mu&);
    static constexpr AccessorVT _vt = {&_type, &_access, &_address};
    using Accessor::Accessor;
};

template <class From, class To>
struct ConstantPointerAcr2 : ConstantPointerAcr0 {
    using AccessorFromType = From;
    using AccessorToType = To;
    Type(* get_type )();
    const To* pointer;
    explicit constexpr ConstantPointerAcr2 (const To* p, uint8 flags = 0) :
        ConstantPointerAcr0(&_vt, flags | ACR_READONLY), get_type(&Type::CppType<To>), pointer(p)
    { }
};

/// reference_func

 // This is a little awkward because we can't transfer the flags from the
 //  calculated Reference's acr to this one.  We'll just have to hope we don't
 //  miss anything important.
struct ReferenceFuncAcr1 : Accessor {
    using Accessor::Accessor;
    static Type _type (const Accessor*, const Mu&);
    static void _access (const Accessor*, AccessOp, Mu&, Callback<void(Mu&)>);
    static Mu* _address (const Accessor*, Mu&);
    static constexpr AccessorVT _vt = {&_type, &_access, &_address};
};
template <class From>
struct ReferenceFuncAcr2 : ReferenceFuncAcr1 {
    using AccessorFromType = From;
    using AccessorToType = Reference;
    Reference(* f )(From&);
    explicit constexpr ReferenceFuncAcr2 (Reference(* f )(From&), uint8 flags = 0) :
        ReferenceFuncAcr1(&_vt, flags), f(f)
    { }
};

} // namespace hacc::in
