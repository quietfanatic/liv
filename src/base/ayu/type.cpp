#include "type.h"

#include <cstdlib>
#include <string>

#include "description.h"
#include "describe.h"

using namespace std::literals;

namespace hacc {

Str Type::name () const {
    if (!desc) return Str();
    return in::get_description_name(desc);
}

const std::type_info& Type::cpp_type () const {
    return *desc->cpp_type;
}

usize Type::cpp_size () const {
    return desc->cpp_size;
}

void Type::default_construct (void* target) const {
    if (!desc->default_construct) throw X::CannotDefaultConstruct(*this);
     // Don't allow constructing objects that can't be destructed
    if (!desc->destruct) throw X::CannotDestruct(*this);
    desc->default_construct(target);
}

void Type::destruct (Mu& p) const {
    if (!desc->destruct) throw X::CannotDestruct(*this);
    desc->destruct(p);
}

void* Type::allocate () const {
    void* p = std::malloc(desc->cpp_size);
    if (!p) throw std::bad_alloc();
    return p;
}

void Type::deallocate (void* p) const {
    std::free(p);
}

Mu* Type::default_new () const {
     // Throw before allocating
    if (!desc->default_construct) throw X::CannotDefaultConstruct(*this);
    if (!desc->destruct) throw X::CannotDestruct(*this);
    void* p = allocate();
    desc->default_construct(p);
    return (Mu*)p;
}

void Type::delete_ (Mu* p) const {
    destruct(*p);
    deallocate(p);
}

Mu* Type::upcast_to (Type to, Mu* p) const {
    if (*this != to) throw X::CannotCoerce(*this, to);
    return p;
}
Mu* Type::downcast_to (Type to, Mu* p) const {
    if (*this != to) throw X::CannotCoerce(*this, to);
    return p;
}
Mu* Type::cast_to (Type to, Mu* p) const {
    if (*this != to) throw X::CannotCoerce(*this, to);
    return p;
}

} using namespace hacc;

HACCABLE(hacc::Type,
    values(
        value(null, Type())
    ),
    delegate(mixed_funcs<std::string>(
        [](const Type& v){
            return std::string(v.name());
        },
        [](Type& v, const std::string& m){
            v = Type(m);
        }
    ))
)

HACCABLE(hacc::X::WrongType,
    elems(
        elem(&X::WrongType::expected),
        elem(&X::WrongType::got)
    )
)
HACCABLE(hacc::X::CannotDefaultConstruct,
    elems( elem(&X::CannotDefaultConstruct::type) )
)
HACCABLE(hacc::X::CannotDestruct,
    elems( elem(&X::CannotDestruct::type) )
)
HACCABLE(hacc::X::CannotCoerce,
    elems(
        elem(&X::CannotCoerce::from),
        elem(&X::CannotCoerce::to)
    )
)

// Testing of Type will be done in dynamic.cpp
