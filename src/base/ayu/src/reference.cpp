#include "../reference.h"

#include "../describe.h"
#include "../dynamic.h"
#include "../resource.h"
#include "../serialize.h"
 // TODO: Is this header needed?
#include "tree-private.h"

namespace ayu {
namespace in {

struct ChainAcr : Accessor {
    const Accessor* a;
    const Accessor* b;
    static Type _type (const Accessor* acr, const Mu& v) {
        auto self = static_cast<const ChainAcr*>(acr);
        Type r;
        self->a->read(v, [&](const Mu& av){
            r = self->b->type(av);
        });
        return r;
    }
    static void _access (const Accessor* acr, AccessOp op, Mu& v, Callback<void(Mu&)> cb) {
        auto self = static_cast<const ChainAcr*>(acr);
        switch (op) {
            case ACR_READ: {
                return self->a->access(ACR_READ, v, [&](Mu& m){
                    self->b->access(ACR_READ, m, cb);
                });
            }
            case ACR_WRITE: {
                 // Have to use modify instead of write here, or other parts of the item
                 //  will get clobbered.  Hope that we don't go down this code path a lot.
                return self->a->access(ACR_MODIFY, v, [&](Mu& m){
                    self->b->access(ACR_WRITE, m, cb);
                });
            }
            case ACR_MODIFY: {
                return self->a->access(ACR_MODIFY, v, [&](Mu& m){
                    self->b->access(ACR_MODIFY, m, cb);
                });
            }
        }
    }
    static Mu* _address (const Accessor* acr, Mu& v) {
        auto self = static_cast<const ChainAcr*>(acr);
        if (self->b->accessor_flags & ACR_ANCHORED_TO_GRANDPARENT) {
            Mu* r = null;
            self->a->access(ACR_READ, v, [&](const Mu& av){
                r = self->b->address(const_cast<Mu&>(av));
            });
            return r;
        }
        else if (auto aa = self->a->address(v)) {
             // We shouldn't get to this codepath but here it is anyway
            return self->b->address(*aa);
        }
        else return null;
    }
    static void _destroy (Accessor* acr) {
        auto self = static_cast<const ChainAcr*>(acr);
        self->a->dec(); self->b->dec();
    }
    static constexpr AccessorVT _vt = {
        &_type, &_access, &_address, null, &_destroy
    };
    explicit ChainAcr (const Accessor* a, const Accessor* b) :
        Accessor(
            &_vt,
             // Readonly if either accessor is readonly
            ((a->accessor_flags & ACR_READONLY)
           | (b->accessor_flags & ACR_READONLY))
             // Anchored to grandparent if both are anchored to grandparent.
          | ((a->accessor_flags & ACR_ANCHORED_TO_GRANDPARENT)
           & (b->accessor_flags & ACR_ANCHORED_TO_GRANDPARENT))
        ), a(a), b(b)
    {
        a->inc(); b->inc();
    }
};

 // TODO: Do we need to do something with accessor_flags for this?
struct AttrFuncAcr : Accessor {
    Reference(* fp )(Mu&, Str);
    String key;
    static Type _type (const Accessor* acr, const Mu& v) {
        auto self = static_cast<const AttrFuncAcr*>(acr);
        return self->fp(const_cast<Mu&>(v), self->key).type();
    }
    static void _access (const Accessor* acr, AccessOp op, Mu& v, Callback<void(Mu&)> cb) {
        auto self = static_cast<const AttrFuncAcr*>(acr);
        self->fp(v, self->key).access(op, cb);
    }
    static Mu* _address (const Accessor* acr, Mu& v) {
        auto self = static_cast<const AttrFuncAcr*>(acr);
        return self->fp(v, self->key).address();
    }
    static void _destroy (Accessor* acr) {
        auto self = static_cast<const AttrFuncAcr*>(acr);
        self->~AttrFuncAcr();
    }
    static constexpr AccessorVT _vt = {
        &_type, &_access, &_address, null, &_destroy
    };
    AttrFuncAcr (Reference(* fp )(Mu&, Str), Str k) : Accessor(&_vt), fp(fp), key(k) { }
};

struct ElemFuncAcr : Accessor {
    Reference(* fp )(Mu&, usize);
    size_t index;
    static Type _type (const Accessor* acr, const Mu& v) {
        auto self = static_cast<const ElemFuncAcr*>(acr);
        return self->fp(const_cast<Mu&>(v), self->index).type();
    }
    static void _access (const Accessor* acr, AccessOp op, Mu& v, Callback<void(Mu&)> cb) {
        auto self = static_cast<const ElemFuncAcr*>(acr);
        self->fp(v, self->index).access(op, cb);
    }
    static Mu* _address (const Accessor* acr, Mu& v) {
        auto self = static_cast<const ElemFuncAcr*>(acr);
        return self->fp(v, self->index).address();
    }
    static constexpr AccessorVT _vt = {&_type, &_access, &_address};
    ElemFuncAcr (Reference(* fp )(Mu&, usize), usize i) : Accessor(&_vt), fp(fp), index(i) { }
};

} using namespace in;

void Reference::require_writable () const {
    if (readonly()) throw X::WriteReadonlyReference(*this);
}

Mu* Reference::require_address () const {
    if (!*this) return null;
    if (auto a = address()) return a;
    else throw X::UnaddressableReference(*this);
}

Reference Reference::chain (const Accessor* o_acr) const {
    if (auto a = address()) return Reference(a, o_acr);
    else return Reference(host, new ChainAcr(acr, o_acr));
}

Reference Reference::chain_attr_func (Reference(* f )(Mu&, Str), Str k) const {
    if (auto a = address()) {
        auto r = f(*a, k);
        if (r) return r;
        else throw X::AttrNotFound(*this, k);
    }
    else {
         // Extra read just to check if the func returns null Reference.
         // If we're here, we're already on a fairly worst-case performance
         //  scenario, so one more check isn't gonna make much difference.
        read([&](const Mu& v){
            Reference ref = f(const_cast<Mu&>(v), k);
            if (!ref) throw X::AttrNotFound(*this, k);
        });
        return Reference(host, new ChainAcr(acr, new AttrFuncAcr(f, k)));
    }
}

Reference Reference::chain_elem_func (Reference(* f )(Mu&, size_t), size_t i) const {
    if (auto a = address()) {
        auto r = f(*a, i);
        if (r) return r;
        else throw X::ElemNotFound(*this, i);
    }
    else {
        read([&](const Mu& v){
            Reference ref = f(const_cast<Mu&>(v), i);
            if (!ref) throw X::ElemNotFound(*this, i);
        });
        return Reference(host, new ChainAcr(acr, new ElemFuncAcr(f, i)));
    }
}

} using namespace ayu;

static Reference empty_reference;

AYU_DESCRIBE(ayu::Reference,
    from_tree([](Reference& v, const Tree&){
        v = Reference();
    }),
    swizzle([](Reference& v, const Tree& t){
        Location loc;
        item_from_tree(&loc, t);
        v = reference_from_location(loc);
    }),
    to_tree([](const Reference& v){
        if (!v) return Tree(null);
        else {
             // Taking address of rvalue should be fine here but the compiler
             // will still complain about it.
            Location loc = reference_to_location(v);
            return item_to_tree(&loc);
        }
    })
)

AYU_DESCRIBE(ayu::X::WriteReadonlyReference,
    delegate(base<X::LogicError>()),
    elems( elem(&X::WriteReadonlyReference::location) )
)
AYU_DESCRIBE(ayu::X::UnaddressableReference,
    delegate(base<X::LogicError>()),
    elems( elem(&X::UnaddressableReference::location) )
)
