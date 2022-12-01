#include "../reference.h"

#include "accessors-private.h"
#include "../describe.h"
#include "../dynamic.h"
#include "../resource.h"
#include "../serialize.h"

namespace ayu {
using namespace in;

void Reference::require_writeable () const {
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

AYU_DESCRIBE(ayu::X::ReferenceError,
    elems(
        elem(base<X::Error>(), inherit),
        elem(&X::ReferenceError::location)
    )
)
AYU_DESCRIBE(ayu::X::WriteReadonlyReference,
    elems(elem(base<X::ReferenceError>(), inherit))
)
AYU_DESCRIBE(ayu::X::UnaddressableReference,
    elems(elem(base<X::ReferenceError>(), inherit))
)
