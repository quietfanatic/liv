// This header provides template ayu descriptions for a few stl types.  The
// corresponding .cpp file provides descriptions for non-template types like
// native integers.  If you want to use things like std::vector in ayu
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
 // contained object itself serializes to null.  Hopefully this won't be a
 // problem.
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::optional<T>),
    desc::name([]{
        static ayu::String r = ayu::cat(ayu::Type::CppType<T>().name(), '?');
        return ayu::Str(r);
    }),
    desc::values(
        desc::value(ayu::null, std::optional<T>())
    ),
    desc::delegate(desc::template ref_func<T>(
        [](std::optional<T>& v)->T&{
            if (!v) v.emplace();
            return *v;
        }
    ))
)

 // std::vector
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::vector<T>),
    desc::name([]{
        using namespace std::literals;
        static ayu::String r = ayu::cat(
            "std::vector<"sv, ayu::Type::CppType<T>().name(), '>'
        );
        return ayu::Str(r);
    }),
    desc::length(desc::template value_funcs<ayu::usize>(
        [](const std::vector<T>& v){ return v.size(); },
        [](std::vector<T>& v, ayu::usize l){ v.resize(l); }
    )),
    desc::elem_func([](std::vector<T>& v, ayu::usize i){
        return ayu::Reference(&v.at(i));
    })
)

 // std::unordered_map with strings for keys.  We might add a more general
 // unordered_map description later.
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::unordered_map<std::string, T>),
    desc::name([]{
        using namespace std::literals;
        static ayu::String r = ayu::cat(
            "std::unordered_map<std::string, "sv,
            ayu::Type::CppType<T>().name(), '>'
        );
        return ayu::Str(r);
    }),
    desc::keys(desc::template mixed_funcs<std::vector<std::string>>(
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
    desc::attr_func([](std::unordered_map<std::string, T>& v, ayu::Str k){
        return ayu::Reference(&v.at(std::string(k)));
    })
)

 // Raw pointers
 // TODO: figure out if we need to do something for const T*
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(T*),
    desc::name([]{
        using namespace std::literals;
        static ayu::String r = ayu::cat(ayu::Type::CppType<T>().name(), '*');
        return ayu::Str(r);
    }),
     // This will probably be faster if we skip the delegate chain, but let's
     // save that until we know we need it.
    desc::delegate(desc::template value_funcs<ayu::Reference>(
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
    desc::name([]{
        using namespace std::literals;
        static ayu::String r = ayu::cat(
            ayu::Type::CppType<T>().name(),
            '[', n, ']'
        );
        return ayu::Str(r);
    }),
    desc::length(desc::template constant<ayu::usize>(n)),
    desc::elem_func([](T(& v )[n], ayu::usize i){
        if (i < n) return ayu::Reference(&v[i]);
        else return ayu::Reference();
    })
)

 // Special case for char[n], mainly to allow string literals to be passed to
 // ayu::dump without surprising behavior.  Note that the deserialization from
 // string WILL NOT NUL-TERMINATE the char[n].
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(ayu::usize n),
    AYU_DESCRIBE_TEMPLATE_TYPE(char[n]),
    desc::name([]{
        using namespace std::literals;
        static ayu::String r = ayu::cat("char["sv, n, ']');
        return ayu::Str(r);
    }),
     // Serialize as a string
    desc::to_tree([](const char(& v )[n]){
         // TODO
        //return ayu::Tree(Str(v, n));
        return ayu::Tree(v);
    }),
     // Deserialize as either a string or an array
    desc::from_tree([](char(& v )[n], const ayu::Tree& tree){
        if (tree.form() == ayu::STRING) {
            auto s = ayu::Str(tree);
            if (s.size() < n) {
                throw ayu::X::TooShort(&v, n, s.size());
            }
            if (s.size() > n) {
                throw ayu::X::TooLong(&v, n, s.size());
            }
            for (uint i = 0; i < n; i++) {
                v[i] = s[i];
            }
        }
        else if (tree.form() == ayu::ARRAY) {
            const auto& a = ayu::Array(tree);
            if (a.size() < n) {
                throw ayu::X::TooShort(&v, n, a.size());
            }
            if (a.size() > n) {
                throw ayu::X::TooLong(&v, n, a.size());
            }
            for (uint i = 0; i < n; i++) {
                v[i] = char(a[i]);
            }
        }
    }),
     // Allow accessing individual elements like an array
    desc::length(desc::template constant<ayu::usize>(n)),
    desc::elem_func([](char(& v )[n], ayu::usize i){
        if (i < n) return ayu::Reference(&v[i]);
        else return ayu::Reference();
    })
)

 // std::array
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T, ayu::usize n),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::array<T, n>),
    desc::name([]{
        using namespace std::literals;
        static ayu::String r = ayu::cat(
            "std::array<"sv + ayu::Type::CppType<T>().name(),
            ", "sv, n, '>'
        );
        return ayu::Str(r);
    }),
    desc::length(desc::template constant<ayu::usize>(n)),
    desc::elem_func([](std::array<T, n>& v, ayu::usize i){
        if (i < n) return ayu::Reference(&v[i]);
        else return ayu::Reference();
    })
)

 // std::pair
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class A, class B),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::pair<A, B>),
    desc::name([]{
        using namespace std::literals;
        static ayu::String r = ayu::cat(
            "std::pair<"sv, ayu::Type::CppType<A>().name(),
            ", "sv, ayu::Type::CppType<B>().name(), '>'
        );
        return ayu::Str(r);
    }),
    desc::elems(
        desc::elem(&std::pair<A, B>::first),
        desc::elem(&std::pair<A, B>::second)
    )
)

 // A bit convoluted but hopefully worth it
namespace ayu::in {
    using namespace std::literals;

     // Recursive template function to construct the type name of the tuple
     // All this just to put commas between the type names.
    template <class... Ts>
    struct TupleNames;
    template <>
    struct TupleNames<> {
        static ayu::String make () {
            return ""s;
        }
    };
    template <class T>
    struct TupleNames<T> {
        static ayu::String make () {
            return ayu::String(Type::CppType<T>().name());
        }
    };
    template <class A, class B, class... Ts>
    struct TupleNames<A, B, Ts...> {
        static ayu::String make () {
            return ayu::cat(
                ayu::Type::CppType<A>().name(),
                ", "sv, TupleNames<B, Ts...>::make()
            );
        }
    };

     // No recursive templates or extra static tables, just expand the parameter
     // pack right inside of elems(...).  We do need to move this out to an
     // external struct though, to receive an index sequence.
    template <class... Ts>
    struct TupleElems {
        using Tuple = std::tuple<Ts...>;
        using desc = ayu::_AYU_DescribeBase<Tuple>;
        template <class T>
        using Getter = T&(*)(Tuple&);
        template <usize... is>
        static constexpr auto make (std::index_sequence<is...>) {
            return desc::elems(
                desc::elem(desc::ref_func(
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
    desc::name([]{
        static_assert(
            (!std::is_reference_v<Ts> && ...),
            "Cannot instantiate AYU description of a tuple with references as type parameters"
        );
        using namespace std::literals;
        static ayu::String r = ayu::cat(
            "std::tuple<"sv, ayu::in::TupleNames<Ts...>::make(), '>'
        );
        return ayu::Str(r);
    }),
    ayu::in::TupleElems<Ts...>::make(
        std::index_sequence_for<Ts...>{}
    )
)
