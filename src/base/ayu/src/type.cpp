#include "../type.h"

#include <cstdlib>
#include <iostream>
#include <typeindex>
#include <string>

#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif

#include "../internal/describe-internal.h"
#include "../describe.h"

using namespace std::literals;

namespace ayu {

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

namespace in {

struct Registry {
    std::unordered_map<std::type_index, const Description*> by_cpp_type;
    std::unordered_map<Str, const Description*> by_name;
    bool initted = false;
};

static Registry& registry () {
    static Registry r;
    return r;
}

static void init_names () {
    auto& r = registry();
    if (!r.initted) {
        r.initted = true;
        for (auto& p : r.by_cpp_type) {
            r.by_name.emplace(get_description_name(p.second), p.second);
        }
    }
}

const Description* register_description (const Description* desc) {
    if (registry().initted) {
        throw X::GenericError("register_description called after init time");
    }
     // TODO: return existing if existing
    auto [p, e] = registry().by_cpp_type.emplace(*desc->cpp_type, desc);
    return p->second;
}

const Description* get_description_by_type_info (const std::type_info& t) {
    auto& ds = registry().by_cpp_type;
    auto iter = ds.find(t);
    if (iter != ds.end()) return iter->second;
    else return null;
}
const Description* need_description_for_type_info (const std::type_info& t) {
    auto desc = get_description_by_type_info(t);
    if (desc) return desc;
    else throw X::UnknownType(t);
}

const Description* get_description_by_name (Str name) {
    init_names();
    auto& ds = registry().by_name;
    auto iter = ds.find(name);
    if (iter != ds.end()) return iter->second;
    else return null;
}
const Description* need_description_for_name (Str name) {
    auto desc = get_description_by_name(name);
    if (desc) return desc;
    else throw X::TypeNotFound(String(name));
}

Str get_description_name (const Description* desc) {
    return desc->name_offset
        ? ((NameDcr<Mu>*)((char*)desc + desc->name_offset))->f()
        : !desc->name.empty() ? desc->name
        : desc->cpp_type->name();
}
bool is_valid_type (const Description* desc) {
    for (auto& p : registry().by_cpp_type) {
        if (p.second == desc) return true;
    }
    return false;
}

void dump_descriptions () {
    for (auto& p : registry().by_cpp_type) {
        std::cerr << p.second->cpp_type->name() << ": "
                  << get_description_name(p.second) << " "
                  << p.second->cpp_size << " "
                  << p.second->default_construct << " "
                  << p.second->destruct << std::endl;
    }
}

std::string get_demangled_name (const std::type_info& t) {
#if __has_include(<cxxabi.h>)
    int status;
    char* demangled = abi::__cxa_demangle(t.name(), nullptr, nullptr, &status);
    if (status != 0) return "(Failed to demangle "s + t.name() + ")"s;
    std::string r = const_cast<const char*>(demangled);
    free(demangled);
    return r;
#else
     // Probably MSVC, which automatically demangles.
    return std::string(t.name());
#endif
}

} using namespace in;
} using namespace ayu;

AYU_DESCRIBE(ayu::Type,
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

AYU_DESCRIBE(ayu::X::UnknownType,
    elems(elem(value_func<std::string>(
        [](const ayu::X::UnknownType& v){ return get_demangled_name(v.cpp_type); }
    )))
)

AYU_DESCRIBE(ayu::X::TypeNotFound,
    elems(elem(&X::TypeNotFound::name))
)

AYU_DESCRIBE(ayu::X::WrongType,
    elems(
        elem(&X::WrongType::expected),
        elem(&X::WrongType::got)
    )
)
AYU_DESCRIBE(ayu::X::CannotDefaultConstruct,
    elems( elem(&X::CannotDefaultConstruct::type) )
)
AYU_DESCRIBE(ayu::X::CannotDestruct,
    elems( elem(&X::CannotDestruct::type) )
)
AYU_DESCRIBE(ayu::X::CannotCoerce,
    elems(
        elem(&X::CannotCoerce::from),
        elem(&X::CannotCoerce::to)
    )
)

// Testing of Type will be done in dynamic.cpp
