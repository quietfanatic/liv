// This header provides template ayu descriptions for a few stl types.  The
// corresponding .cpp file provides descriptions for non-template types like
// builtin integers.  If you want to use things like std::vector in ayu
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
#include "describe-base.h"
#include "reference.h"

 // std::optional serializes to null for nullopt and whatever it contains
 // otherwise.  Yes, that means that this won't serialize properly if the
 // contained object itself serializes to null.  Hopefully this won't be
 // a problem.
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::optional<T>),
    hcb::name([]{
        using namespace ayu;
        using namespace std::literals;
        static String r = Type::CppType<T>().name() + "?"s;
        return Str(r);
    }),
    hcb::values(
        hcb::value(ayu::null, std::optional<T>())
    ),
    hcb::delegate(hcb::template ref_func<T>(
        [](std::optional<T>& v)->T&{ return *v; }
    ))
)

 // std::vector
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::vector<T>),
    hcb::name([]{
        using namespace ayu;
        using namespace std::literals;
        static String r = "std::vector<"s + Type::CppType<T>().name() + ">"s;
        return Str(r);
    }),
    hcb::length(hcb::template value_funcs<ayu::usize>(
        [](const std::vector<T>& v){ return v.size(); },
        [](std::vector<T>& v, ayu::usize l){ v.resize(l); }
    )),
    hcb::elem_func([](std::vector<T>& v, ayu::usize i){
        return ayu::Reference(&v.at(i));
    })
)

 // std::unordered_map
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::unordered_map<std::string, T>),
    hcb::name([]{
        using namespace ayu;
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
        [](std::unordered_map<std::string, T>& v,
           const std::vector<std::string>& ks
        ){
            v.clear();
            for (auto& k : ks) {
                v.emplace(k, T());
            }
        }
    )),
    hcb::attr_func([](std::unordered_map<std::string, T>& v, ayu::Str k){
        return ayu::Reference(&v.at(std::string(k)));
    })
)

 // Raw pointers
 // TODO: figure out if we need to do something for const T*
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(T*),
    hcb::name([]{
        using namespace ayu;
        using namespace std::literals;
        static String r = Type::CppType<T>().name() + "*"s;
        return Str(r);
    }),
     // This will probably be faster if we skip the delegate chain, but let's
     // save that until we know we need it.
    hcb::delegate(hcb::template value_funcs<ayu::Reference>(
        [](T* const& v){
            return ayu::Reference(v);
        },
        [](T*& v, ayu::Reference r){
            if (!r) v = ayu::null;
            else v = r.require_address_as<T>();
        }
    ))
)

 // Raw arrays T[n] - I can't believe this works
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T, ayu::usize n),
    AYU_DESCRIBE_TEMPLATE_TYPE(T[n]),
    hcb::name([]{
        using namespace ayu;
        using namespace std::literals;
        static String r = Type::CppType<T>().name()
                        + "["sv + std::to_string(n) + "]"sv;
        return Str(r);
    }),
    hcb::length(hcb::template constant<ayu::usize>(n)),
    hcb::elem_func([](T(& v )[n], ayu::usize i){
        if (i < n) return ayu::Reference(&v[i]);
        else return ayu::Reference();
    })
)

 // Special case for const char[n], which is probably a string literal.
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(ayu::usize n),
    AYU_DESCRIBE_TEMPLATE_TYPE(char[n]),
    hcb::name([]{
        using namespace ayu;
        using namespace std::literals;
        static String r = "char[" + std::to_string(n) + "]"sv;
        return Str(r);
    }),
     // Serialize as a string
    hcb::to_tree([](const char(& v )[n]){
        return ayu::Tree(v);
    }),
     // Deserialize as either a string or an array
    hcb::from_tree([](char(& v )[n], const ayu::Tree& tree){
        if (tree.form() == ayu::STRING) {
            auto s = ayu::Str(tree);
            if (s.size() != n) {
                throw ayu::X::WrongLength(&v, n, n, s.size());
            }
            for (uint i = 0; i < n; i++) {
                v[i] = s[i];
            }
        }
        else if (tree.form() == ayu::ARRAY) {
            const auto& a = ayu::Array(tree);
            if (a.size() != n) {
                throw ayu::X::WrongLength(&v, n, n, a.size());
            }
            for (uint i = 0; i < n; i++) {
                v[i] = char(a[i]);
            }
        }
    }),
     // Allow accessing individual elements like an array
    hcb::length(hcb::template constant<ayu::usize>(n)),
    hcb::elem_func([](char(& v )[n], ayu::usize i){
        if (i < n) return ayu::Reference(&v[i]);
        else return ayu::Reference();
    })
)

 // std::array
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T, ayu::usize n),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::array<T, n>),
    hcb::name([]{
        using namespace ayu;
        using namespace std::literals;
        static String r = "std::array<"sv + Type::CppType<T>().name()
                        + ", "sv + std::to_string(n) + ">"sv;
        return Str(r);
    }),
    hcb::length(hcb::template constant<ayu::usize>(n)),
    hcb::elem_func([](std::array<T, n>& v, ayu::usize i){
        if (i < n) return ayu::Reference(&v[i]);
        else return ayu::Reference();
    })
)


AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class A, class B),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::pair<A, B>),
    hcb::name([]{
        using namespace ayu;
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
namespace ayu::in {
    using namespace ayu;
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
        using hcb = ayu::DescribeBase<Tuple>;
        template <class T>
        using Getter = T&(*)(Tuple&);
        template <usize... is>
        static constexpr auto make (std::index_sequence<is...>) {
            return hcb::elems(
                hcb::elem(hcb::ref_func(
                    Getter<typename std::tuple_element<is, Tuple>::type>(
                        &std::get<is, Ts...>
                    )
                ))...
            );
        }
    };
}

 // Note that although std::tuple removes references from its members,
 // Ts... is still stuck with references if it has them.  So please
 // std::remove_cvref on the params before instantiating this.
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class... Ts),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::tuple<Ts...>),
    hcb::name([]{
        using namespace ayu;
        using namespace std::literals;
        static String r = "std::tuple<"sv
            + ayu::in::TupleNames<Ts...>::make()
            + ">"sv;
        return Str(r);
    }),
    ayu::in::TupleElems<Ts...>::make(
        std::index_sequence_for<Ts...>{}
    )
)
