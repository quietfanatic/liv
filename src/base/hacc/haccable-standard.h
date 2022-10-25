// This header provides template haccabilities for a few stl types.  The
// corresponding .cpp file provides haccabilities for non-template types like
// builtin integers.  If you want to use things like std::vector in haccable
// descriptions, include this file.

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "common.h"
#include "haccable.h"
#include "reference.h"

HACCABLE_TEMPLATE(
    HACCABLE_TEMPLATE_PARAMS(class T),
    HACCABLE_TEMPLATE_TYPE(std::optional<T>),
    hcb::name([]{
        using namespace hacc;
        using namespace std::literals;
        static String r = Type::CppType<T>().name() + "?"s;
        return Str(r);
    }),
    hcb::values(
        hcb::value(hacc::null, std::optional<T>())
    ),
    hcb::delegate(hcb::template ref_func<T>(
        [](std::optional<T>& v)->T&{ return *v; }
    ))
)

HACCABLE_TEMPLATE(
    HACCABLE_TEMPLATE_PARAMS(class T),
    HACCABLE_TEMPLATE_TYPE(std::vector<T>),
    hcb::name([]{
        using namespace hacc;
        using namespace std::literals;
        static String r = "std::vector<"s + Type::CppType<T>().name() + ">"s;
        return Str(r);
    }),
    hcb::length(hcb::template value_funcs<hacc::usize>(
        [](const std::vector<T>& v){ return v.size(); },
        [](std::vector<T>& v, hacc::usize l){ v.resize(l); }
    )),
    hcb::elem_func([](std::vector<T>& v, hacc::usize i){
        return hacc::Reference(&v.at(i));
    })
)

HACCABLE_TEMPLATE(
    HACCABLE_TEMPLATE_PARAMS(class T),
    HACCABLE_TEMPLATE_TYPE(std::unordered_map<std::string, T>),
    hcb::name([]{
        using namespace hacc;
        using namespace std::literals;
        static String r = "std::unordered_map<std::string, "s
            + Type::CppType<T>().name() + ">"s;
        return Str(r);
    }),
    hcb::keys(hcb::template mixed_funcs<std::vector<std::string>>(
        [](const std::unordered_map<std::string, T>& v){
            std::vector<std::string> r;
            for (auto& p : v) {
                r.emplace_back(p.first);
            }
            return r;
        },
        [](std::unordered_map<std::string, T>& v, const std::vector<std::string>& ks){
            v.clear();
            for (auto& k : ks) {
                v.emplace(k, T());
            }
        }
    )),
    hcb::attr_func([](std::unordered_map<std::string, T>& v, hacc::Str k){
        return hacc::Reference(&v.at(std::string(k)));
    })
)

 // TODO: figure out if we need to do something for const T*
HACCABLE_TEMPLATE(
    HACCABLE_TEMPLATE_PARAMS(class T),
    HACCABLE_TEMPLATE_TYPE(T*),
    hcb::name([]{
        using namespace hacc;
        using namespace std::literals;
        static String r = Type::CppType<T>().name() + "*"s;
        return Str(r);
    }),
     // This will probably be faster if we skip the delegate chain, but let's
     //  save that until we know we need it.
    hcb::delegate(hcb::template value_funcs<hacc::Reference>(
        [](T* const& v){
            return hacc::Reference(v);
        },
        [](T*& v, hacc::Reference r){
            if (!r) v = hacc::null;
            else v = r.require_address_as<T>();
        }
    ))
)

 // I can't believe this works
HACCABLE_TEMPLATE(
    HACCABLE_TEMPLATE_PARAMS(class T, hacc::usize n),
    HACCABLE_TEMPLATE_TYPE(T[n]),
    hcb::name([]{
        using namespace hacc;
        using namespace std::literals;
        static String r = Type::CppType<T>().name() + "["s + std::to_string(n) + "]"s;
        return Str(r);
    }),
    hcb::length(hcb::template constant<hacc::usize>(n)),
    hcb::elem_func([](T(&v)[n], hacc::usize i){
        if (i < n) return hacc::Reference(&v[i]);
        else return hacc::Reference();
    })
)

