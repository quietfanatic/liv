// This header provides template haccabilities for a few stl types.  The
// corresponding .cpp file provides haccabilities for non-template types like
// builtin integers.  If you want to use things like std::vector in haccable
// descriptions, include this file.

#pragma once

#include <array>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common.h"
#include "haccable.h"
#include "reference.h"

 // std::optional haccifies to null for nullopt and whatever it contains
 // otherwise.  Yes, that means that this won't serialize properly if the
 // contained object itself serializes to null.  Hopefully this won't be
 // a problem.
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
        static String r = Type::CppType<T>().name()
                        + "["sv + std::to_string(n) + "]"sv;
        return Str(r);
    }),
    hcb::length(hcb::template constant<hacc::usize>(n)),
    hcb::elem_func([](T(&v)[n], hacc::usize i){
        if (i < n) return hacc::Reference(&v[i]);
        else return hacc::Reference();
    })
)

HACCABLE_TEMPLATE(
    HACCABLE_TEMPLATE_PARAMS(class T, hacc::usize n),
    HACCABLE_TEMPLATE_TYPE(std::array<T, n>),
    hcb::name([]{
        using namespace hacc;
        using namespace std::literals;
        static String r = "std::array<"sv + Type::CppType<T>().name()
                        + ", "sv + std::to_string(n) + ">"sv;
        return Str(r);
    }),
    hcb::length(hcb::template constant<hacc::usize>(n)),
    hcb::elem_func([](std::array<T, n>& v, hacc::usize i){
        if (i < n) return hacc::Reference(&v[i]);
        else return hacc::Reference();
    })
)

HACCABLE_TEMPLATE(
    HACCABLE_TEMPLATE_PARAMS(class A, class B),
    HACCABLE_TEMPLATE_TYPE(std::pair<A, B>),
    hcb::name([]{
        using namespace hacc;
        using namespace std::literals;
        static String r = "std::pair<"sv + Type::CppType<A>().name()
                        + ", "sv + Type::CppType<B>().name() + ">"sv;
        return Str(r);
    }),
    hcb::elems(
        hcb::elem(&std::pair<A, B>::first),
        hcb::elem(&std::pair<A, B>::second)
    )
)

 // A bit convoluted but hopefully worth it
namespace hacc::in {
    using namespace hacc;
    using namespace std::literals;

     // Recursive template function to construct the type name of the tuple
     // All this just to put commas between the type names.
    template <class... Ts>
    struct TupleNames;
    template <>
    struct TupleNames<> {
        static String make () {
            return "";
        }
    };
    template <class T>
    struct TupleNames<T> {
        static String make () {
            return String(Type::CppType<T>().name());
        }
    };
    template <class A, class B, class... Ts>
    struct TupleNames<A, B, Ts...> {
        static String make () {
            return Type::CppType<A>().name()
                 + ", "s + TupleNames<B, Ts...>::make();
        }
    };

     // No recursive templates or extra static tables, just expand the parameter
     // pack right inside of elems(...).  We do need to move this out to an
     // external struct though, to receive an index sequence.
    template <class... Ts>
    struct TupleElems {
        using Tuple = std::tuple<Ts...>;
        using hcb = hacc::HaccabilityBase<Tuple>;
        template <class T>
        using Getter = T&(*)(Tuple&);
        template <usize... is>
        static constexpr auto make (std::index_sequence<is...>) {
            return hcb::elems(
                hcb::elem(hcb::ref_func(
                    Getter<typename std::tuple_element<is, Tuple>::type>(&std::get<is, Ts...>)
                ))...
            );
        }
    };
}

 // Note that although std::tuple removes references from its members,
 // Ts... is still stuck with references if it has them.  So please
 // std::remove_cvref on the params before instantiating this.
HACCABLE_TEMPLATE(
    HACCABLE_TEMPLATE_PARAMS(class... Ts),
    HACCABLE_TEMPLATE_TYPE(std::tuple<Ts...>),
    hcb::name([]{
        using namespace hacc;
        using namespace std::literals;
        static String r = "std::tuple<"sv
            + hacc::in::TupleNames<Ts...>::make()
            + ">"sv;
        return Str(r);
    }),
    hacc::in::TupleElems<Ts...>::make(
        std::index_sequence_for<Ts...>{}
    )
)
