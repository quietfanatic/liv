#include "registry.h"

#include <iostream>
#include <typeindex>

#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif

#include "description.h"
#include "haccable.h"

using namespace std::string_literals;

namespace hacc::in {

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
    auto ds = registry().by_cpp_type;
    auto iter = ds.find(t);
    if (iter != ds.end()) return iter->second;
    else return null;
}
const Description* need_description_for_type_info (const std::type_info& t) {
    auto desc = get_description_by_type_info(t);
    if (desc) return desc;
    else throw X::Unhaccable(t);
}

const Description* get_description_by_name (Str name) {
    init_names();
    auto ds = registry().by_name;
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
    for (auto p : registry().by_cpp_type) {
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

} using namespace hacc::in;
using namespace hacc;

HACCABLE(hacc::X::Unhaccable,
    elems(elem(value_func<std::string>(
        [](const hacc::X::Unhaccable& v){ return get_demangled_name(v.cpp_type); }
    )))
)

HACCABLE(hacc::X::TypeNotFound,
    elems(elem(&X::TypeNotFound::name))
)
