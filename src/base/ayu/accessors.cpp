#include "accessors.h"

#include "reference.h"

namespace ayu::in {

Type MemberAcr0::_type (const Accessor* acr, const Mu&) {
    auto self = static_cast<const MemberAcr2<Mu, Mu>*>(acr);
    return self->get_type();
}
void MemberAcr0::_access (const Accessor* acr, AccessOp, Mu& from, Callback<void(Mu&)> cb) {
    auto self = static_cast<const MemberAcr2<Mu, Mu>*>(acr);
    cb(from.*(self->mp));
}
Mu* MemberAcr0::_address (const Accessor* acr, Mu& from) {
    auto self = static_cast<const MemberAcr2<Mu, Mu>*>(acr);
    return &(from.*(self->mp));
}

Type RefFuncAcr0::_type (const Accessor* acr, const Mu&) {
    auto self = static_cast<const RefFuncAcr2<Mu, Mu>*>(acr);
    return self->get_type();
}
void RefFuncAcr0::_access (const Accessor* acr, AccessOp, Mu& from, Callback<void(Mu&)> cb) {
    auto self = static_cast<const RefFuncAcr2<Mu, Mu>*>(acr);
    cb((self->f)(from));
}
Mu* RefFuncAcr0::_address (const Accessor* acr, Mu& from) {
     // It's the programmer's responsibility to know whether they're
     //  allowed to do this or not.
    auto self = static_cast<const RefFuncAcr2<Mu, Mu>*>(acr);
    return &(self->f)(from);
}

Type ConstRefFuncAcr0::_type (const Accessor* acr, const Mu&) {
    auto self = static_cast<const ConstRefFuncAcr2<Mu, Mu>*>(acr);
    return self->get_type();
}
void ConstRefFuncAcr0::_access (const Accessor* acr, AccessOp op, Mu& from, Callback<void(Mu&)> cb) {
    if (op != ACR_READ) throw X::WriteReadonlyAccessor();
    auto self = static_cast<const ConstRefFuncAcr2<Mu, Mu>*>(acr);
    cb(const_cast<Mu&>((self->f)(from)));
}
Mu* ConstRefFuncAcr0::_address (const Accessor* acr, Mu& from) {
    auto self = static_cast<const ConstRefFuncAcr2<Mu, Mu>*>(acr);
    return const_cast<Mu*>(&(self->f)(from));
}

Type ConstantPointerAcr0::_type (const Accessor* acr, const Mu&) {
    auto self = static_cast<const ConstantPointerAcr2<Mu, Mu>*>(acr);
    return self->get_type();
}
void ConstantPointerAcr0::_access (const Accessor* acr, AccessOp op, Mu&, Callback<void(Mu&)> cb) {
    if (op != ACR_READ) throw X::WriteReadonlyAccessor();
    auto self = static_cast<const ConstantPointerAcr2<Mu, Mu>*>(acr);
    cb(*const_cast<Mu*>(self->pointer));
}

 // Kind of a workaround for an unworkable situation
static constexpr Null null_ref_value = null;

Type ReferenceFuncAcr1::_type (const Accessor* acr, const Mu& from) {
    auto self = static_cast<const ReferenceFuncAcr2<Mu>*>(acr);
    auto ref = self->f(const_cast<Mu&>(from));
    if (ref.empty()) ref = &null_ref_value;
    return ref.type();
}
void ReferenceFuncAcr1::_access (const Accessor* acr, AccessOp op, Mu& from, Callback<void(Mu&)> cb) {
    auto self = static_cast<const ReferenceFuncAcr2<Mu>*>(acr);
    auto ref = self->f(from);
    if (ref.empty()) ref = &null_ref_value;
    ref.access(op, cb);
}
Mu* ReferenceFuncAcr1::_address (const Accessor* acr, Mu& from) {
    auto self = static_cast<const ReferenceFuncAcr2<Mu>*>(acr);
    auto ref = self->f(from);
    if (ref.empty()) return null;
    return ref.address();
}

} using namespace ayu::in;
using namespace ayu;

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"

namespace ayu::in {
     // For making sure deduction works.  Won't bother making this for other Acrs.
    template <class From, class To>
    MemberAcr2<From, To> deduce_MemberAcr (To From::* mp) {
        return MemberAcr2<From, To>{mp};
    }
}

static tap::TestSet tests ("base/ayu/accessors", []{
    using namespace tap;
    struct Thing {
        int a;
        int b;
    };
    struct SubThing : Thing {
        int c;
    };
    SubThing thing2 {7, 8, 9};

    BaseAcr2<SubThing, Thing>{}.read(reinterpret_cast<const Mu&>(thing2), [&](const Mu& thing){
        is(reinterpret_cast<const Thing&>(thing).b, 8, "BaseAcr::read");
    });
    BaseAcr2<SubThing, Thing>{}.write(reinterpret_cast<Mu&>(thing2), [&](Mu& thing){
        auto& th = reinterpret_cast<Thing&>(thing);
        th.a = 77;
        th.b = 88;
    });
    is(thing2.b, 88, "BaseAcr::write");

    auto test_addressable = [&](const char* type, auto acr){
        Thing t {1, 2};
        is(
            acr.address(reinterpret_cast<Mu&>(t)),
            reinterpret_cast<Mu*>(&t.b),
            (type + "::address"s).c_str()
        );
        acr.read(reinterpret_cast<const Mu&>(t), [&](const Mu& v){
            is(reinterpret_cast<const int&>(v), 2, (type + "::read"s).c_str());
        });
        acr.write(reinterpret_cast<Mu&>(t), [&](Mu& v){
            reinterpret_cast<int&>(v) = 4;
        });
        is(t.b, 4, (type + "::write"s).c_str());
        acr.modify(reinterpret_cast<Mu&>(t), [&](Mu& v){
            reinterpret_cast<int&>(v) += 5;
        });
        is(t.b, 9, (type + "::modify"s).c_str());
    };
    auto test_unaddressable = [&](const char* type, auto acr){
        Thing t {1, 2};
        is(
            acr.address(reinterpret_cast<Mu&>(t)),
            null,
            (type + "::address return null"s).c_str()
        );
        acr.read(reinterpret_cast<const Mu&>(t), [&](const Mu& v){
            is(reinterpret_cast<const int&>(v), 2, (type + "::read"s).c_str());
        });
        acr.write(reinterpret_cast<Mu&>(t), [&](Mu& v){
            reinterpret_cast<int&>(v) = 4;
        });
        is(t.b, 4, (type + "::write"s).c_str());
        acr.modify(reinterpret_cast<Mu&>(t), [&](Mu& v){
            reinterpret_cast<int&>(v) += 5;
        });
        is(t.b, 9, (type + "::modify"s).c_str());
    };

    test_addressable("MemberAcr", deduce_MemberAcr(&Thing::b));
    test_addressable("RefFuncAcr", RefFuncAcr2<Thing, int>{
        [](Thing& t)->int&{ return t.b; }
    });
    test_unaddressable("RefFuncsAcr", RefFuncsAcr2<Thing, int>{
        [](const Thing& t)->const int&{ return t.b; },
        [](Thing& t, const int& v){ t.b = v; }
    });
    test_unaddressable("ValueFuncsAcr", ValueFuncsAcr2<Thing, int>{
        [](const Thing& t)->int{ return t.b; },
        [](Thing& t, int v){ t.b = v; }
    });
    test_unaddressable("MixedFuncsAcr", MixedFuncsAcr2<Thing, int>{
        [](const Thing& t)->int{ return t.b; },
        [](Thing& t, const int& v){ t.b = v; }
    });
    done_testing();
});
#endif
