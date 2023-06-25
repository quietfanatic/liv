#include "../reference.h"

#include "accessors-private.h"
#include "descriptors-private.h"
#include "../describe.h"
#include "../dynamic.h"
#include "../resource.h"
#include "../scan.h"
#include "../serialize.h"

namespace ayu {
using namespace in;

void Reference::require_writeable () const {
    if (readonly()) {
        throw X<WriteReadonlyReference>(reference_to_location(*this), type());
    }
}

Mu* Reference::require_address () const {
    if (!*this) return null;
    if (auto a = address()) return a;
    else throw X<UnaddressableReference>(reference_to_location(*this), type());
}

Reference Reference::chain (const Accessor* o_acr) const {
    if (auto a = address()) {
        return Reference(Pointer(a, type()), o_acr);
    }
    else {
        return Reference(host, new ChainAcr(acr, o_acr));
    }
}

Reference Reference::chain_attr_func (Reference(* f )(Mu&, AnyString), AnyString k) const {
    if (auto a = address()) {
        auto r = f(*a, k);
        if (r) return r;
        else throw X<AttrNotFound>(reference_to_location(*this), type(), move(k));
    }
    else {
         // Extra read just to check if the func returns null Reference.
         // If we're here, we're already on a fairly worst-case performance
         //  scenario, so one more check isn't gonna make much difference.
        read([&](const Mu& v){
            Reference ref = f(const_cast<Mu&>(v), k);
            if (!ref) {
                throw X<AttrNotFound>(reference_to_location(*this), type(), move(k));
            }
        });
        return Reference(host, new ChainAcr(acr, new AttrFuncAcr(f, move(k))));
    }
}

Reference Reference::chain_elem_func (Reference(* f )(Mu&, size_t), size_t i) const {
    if (auto a = address()) {
        auto r = f(*a, i);
        if (r) return r;
        else throw X<ElemNotFound>(reference_to_location(*this), type(), i);
    }
    else {
        read([&](const Mu& v){
            Reference ref = f(const_cast<Mu&>(v), i);
            if (!ref) {
                throw X<ElemNotFound>(reference_to_location(*this), type(), i);
            }
        });
        return Reference(host, new ChainAcr(acr, new ElemFuncAcr(f, i)));
    }
}

} using namespace ayu;

AYU_DESCRIBE(ayu::Reference,
     // Can't use delegate with &reference_to_location, because the call to
     // reference_to_location will trigger a scan, which will try to follow the
     // delegation by calling reference_to_location, ad inifinitum.  This does
     // mean you can't have a Reference pointing to a Location that is actually
     // a Reference.  Which...well, if you get to the point where you're trying
     // to do that, you should probably refactor anyway, after seeing a doctor.
    to_tree([](const Reference& ref){
        if (ref) {
            Location loc = reference_to_location(ref);
            return item_to_tree(&loc, current_location());
        }
        else return Tree(null);
    }),
    from_tree([](Reference& v, const Tree&){
        v = Reference();
    }),
    swizzle([](Reference& v, const Tree& t){
        if (t.form != NULLFORM) {
            Location loc;
             // DELAY_SWIZZLE enables cyclic references.
            item_from_tree(&loc, t, current_location(), DELAY_SWIZZLE);
            v = reference_from_location(loc);
        }
    })
)

AYU_DESCRIBE(ayu::ReferenceError,
    elems(
        elem(base<Error>(), inherit),
        elem(&ReferenceError::location),
        elem(&ReferenceError::type)
    )
)
AYU_DESCRIBE(ayu::WriteReadonlyReference,
    elems(elem(base<ReferenceError>(), inherit))
)
AYU_DESCRIBE(ayu::UnaddressableReference,
    elems(elem(base<ReferenceError>(), inherit))
)
