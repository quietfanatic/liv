// This header provides template ayu descriptions for a few stl types.  The
// corresponding .cpp file provides descriptions for non-template types like
// native integers.  If you want to use things like std::vector in ayu
// descriptions, include this file.

#pragma once

#include <array>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../uni/string.h"
#include "common.h"
#include "exception.h"
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
        static std::string r = uni::old_cat(ayu::Type::CppType<T>().name(), '?');
        return uni::OldStr(r);
    }),
    desc::values(
        desc::value(ayu::null, std::optional<T>())
    ),
    desc::delegate(desc::template ref_func<T>(
     // This aggressively de-nulls the optional.  Is that what we want to do?
        [](std::optional<T>& v)->T&{
            if (!v) v.emplace();
            return *v;
        }
    ))
)

 // uni arrays
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(uni::UniqueArray<T>),
    desc::name([]{
        using namespace std::literals;
        static std::string r = uni::old_cat(
            "uni::UniqueArray<"sv, ayu::Type::CppType<T>().name(), '>'
        );
        return uni::OldStr(r);
    }),
    desc::length(desc::template value_methods<
        uni::usize, &uni::UniqueArray<T>::size, &uni::UniqueArray<T>::resize
    >()),
    desc::elem_func([](uni::UniqueArray<T>& v, uni::usize i){
        return i < v.size() ? ayu::Reference(&v[i]) : ayu::Reference();
    })
)
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(uni::SharedArray<T>),
    desc::name([]{
        using namespace std::literals;
        static std::string r = uni::old_cat(
            "uni::SharedArray<"sv, ayu::Type::CppType<T>().name(), '>'
        );
        return uni::OldStr(r);
    }),
    desc::length(desc::template value_methods<
        uni::usize, &uni::SharedArray<T>::size, &uni::SharedArray<T>::resize
    >()),
    desc::elem_func([](uni::SharedArray<T>& v, uni::usize i){
        return i < v.size() ? ayu::Reference(&v[i]) : ayu::Reference();
    })
)
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(uni::AnyArray<T>),
    desc::name([]{
        using namespace std::literals;
        static std::string r = uni::old_cat(
            "uni::AnyArray<"sv, ayu::Type::CppType<T>().name(), '>'
        );
        return uni::OldStr(r);
    }),
    desc::length(desc::template value_methods<
        uni::usize, &uni::AnyArray<T>::size, &uni::AnyArray<T>::resize
    >()),
    desc::elem_func([](uni::AnyArray<T>& v, uni::usize i){
        return i < v.size() ? ayu::Reference(&v[i]) : ayu::Reference();
    })
)

 // std::vector
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::vector<T>),
    desc::name([]{
        using namespace std::literals;
        static std::string r = uni::old_cat(
            "std::vector<"sv, ayu::Type::CppType<T>().name(), '>'
        );
        return uni::OldStr(r);
    }),
    desc::length(desc::template value_methods<
        uni::usize, &std::vector<T>::size, &std::vector<T>::resize
    >()),
    desc::elem_func([](std::vector<T>& v, uni::usize i){
        return i < v.size() ? ayu::Reference(&v[i]) : ayu::Reference();
    })
)

 // std::unordered_map with strings for keys.  We might add a more general
 // unordered_map description later.
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::unordered_map<std::string, T>),
    desc::name([]{
        using namespace std::literals;
        static std::string r = uni::old_cat(
            "std::unordered_map<std::string, "sv,
            ayu::Type::CppType<T>().name(), '>'
        );
        return uni::OldStr(r);
    }),
    desc::keys(desc::template mixed_funcs<std::vector<std::string_view>>(
        [](const std::unordered_map<std::string, T>& v){
            std::vector<std::string_view> r;
            for (auto& p : v) {
                r.emplace_back(p.first);
            }
            return r;
        },
        [](std::unordered_map<std::string, T>& v,
           const std::vector<std::string_view>& ks
        ){
            v.clear();
            for (auto& k : ks) {
                v.emplace(k, T());
            }
        }
    )),
    desc::attr_func([](std::unordered_map<std::string, T>& v, uni::OldStr k){
        auto iter = v.find(k);
        return iter != v.end()
            ? ayu::Reference(&iter->second)
            : ayu::Reference();
    })
)

 // std::map with strings for keys.  We might add a more general map description
 // later.
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::map<std::string, T>),
    desc::name([]{
        using namespace std::literals;
        static std::string r = uni::old_cat(
            "std::map<std::string, "sv, ayu::Type::CppType<T>().name(), '>'
        );
        return uni::OldStr(r);
    }),
    desc::keys(desc::template mixed_funcs<std::vector<std::string_view>>(
        [](const std::map<std::string, T>& v){
            std::vector<std::string_view> r;
            for (auto& p : v) {
                r.emplace_back(p.first);
            }
            return r;
        },
        [](std::map<std::string, T>& v,
           const std::vector<std::string_view>& ks
        ){
            v.clear();
            for (auto& k : ks) {
                v.emplace(k, T());
            }
        }
    )),
    desc::attr_func([](std::map<std::string, T>& v, uni::OldStr k){
        auto iter = v.find(k);
        return iter != v.end()
            ? ayu::Reference(&iter->second)
            : ayu::Reference();
    })
)

 // std::unordered_set
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::unordered_set<T>),
    desc::name([]{
        using namespace std::literals;
        static std::string r = uni::old_cat(
            "std::unordered_set<"sv,
            ayu::Type::CppType<T>().name(), '>'
        );
        return uni::OldStr(r);
    }),
     // Although sets serialize to arrays, accessing elements in a set by index
     // doesn't make sense, so use to_tree and from_tree instead of length and
     // elem_func
    desc::to_tree([](const std::unordered_set<T>& v){
        ayu::TreeArray a;
        for (auto& e : v) {
            a.emplace_back(ayu::item_to_tree(e));
        }
        return ayu::Tree(std::move(a));
    }),
    desc::from_tree([](std::unordered_set<T>& v, const ayu::Tree& tree){
        if (tree.form != ayu::ARRAY) {
            throw ayu::X<ayu::InvalidForm>(ayu::current_location(), tree);
        }
        auto a = ayu::TreeArraySlice(tree);
        v.reserve(a.size());
         // If we use C++17's node interface, we can get away with not doing a
         // move() on the element, thereby supporting elements without a move
         // constructor.
        std::unordered_set<T> source;
        auto loc = ayu::current_location();
        uni::usize i = 0;
        for (auto& e : a) {
            auto iter = source.emplace().first;
            auto node = source.extract(iter);
             // This Location value isn't really correct because the order of
             // the elements can change, but it's needed if there are References
             // inside the elements.
            item_from_tree(&node.value(), e, ayu::Location(loc, i++));
            auto res = v.insert(std::move(node));
             // Check for duplicates.
            if (!res.inserted) {
                throw ayu::X<ayu::GenericError>{uni::old_cat(
                    "Duplicate element given for ",
                    ayu::Type::CppType<std::unordered_set<T>>().name()
                )};
            }
        }
    })
)

 // std::set.  Same as std::unordered_set above, but elements will be serialized
 // in sorted order.
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::set<T>),
    desc::name([]{
        using namespace std::literals;
        static std::string r = uni::old_cat(
            "std::set<"sv, ayu::Type::CppType<T>().name(), '>'
        );
        return uni::OldStr(r);
    }),
    desc::to_tree([](const std::set<T>& v){
        ayu::TreeArray a;
        for (auto& e : v) {
            a.emplace_back(ayu::item_to_tree(&e));
        }
        return ayu::Tree(std::move(a));
    }),
    desc::from_tree([](std::set<T>& v, const ayu::Tree& tree){
        if (tree.form != ayu::ARRAY) {
            throw ayu::X<ayu::InvalidForm>(ayu::current_location(), tree);
        }
        auto a = ayu::TreeArraySlice(tree);
        std::set<T> source;
        auto loc = ayu::current_location();
        uni::usize i = 0;
        for (auto& e : a) {
            auto iter = source.emplace().first;
            auto node = source.extract(iter);
            item_from_tree(&node.value(), e, ayu::Location(loc, i++));
            auto res = v.insert(std::move(node));
             // Check for duplicates.
            if (!res.inserted) {
                throw ayu::X<ayu::GenericError>{uni::old_cat(
                    "Duplicate element given for ",
                    ayu::Type::CppType<std::set<T>>().name()
                )};
            }
        }
    })
)

 // Raw pointers
 // TODO: figure out if we need to do something for const T*
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(T*),
    desc::name([]{
        using namespace std::literals;
        static std::string r = uni::old_cat(ayu::Type::CppType<T>().name(), '*');
        return uni::OldStr(r);
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
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T, uni::usize n),
    AYU_DESCRIBE_TEMPLATE_TYPE(T[n]),
    desc::name([]{
        using namespace std::literals;
        static std::string r = uni::old_cat(
            ayu::Type::CppType<T>().name(),
            '[', n, ']'
        );
        return uni::OldStr(r);
    }),
    desc::length(desc::template constant<uni::usize>(n)),
    desc::elem_func([](T(& v )[n], uni::usize i){
        if (i < n) return ayu::Reference(&v[i]);
        else return ayu::Reference();
    })
)

 // Special case for char[n], mainly to allow string literals to be passed to
 // ayu::dump without surprising behavior.  Note that the deserialization from
 // string WILL NOT NUL-TERMINATE the char[n].
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(uni::usize n),
    AYU_DESCRIBE_TEMPLATE_TYPE(char[n]),
    desc::name([]{
        using namespace std::literals;
        static std::string r = uni::old_cat("char["sv, n, ']');
        return uni::OldStr(r);
    }),
     // Serialize as a string
    desc::to_tree([](const char(& v )[n]){
        return ayu::Tree(OldStr(v, n));
    }),
     // Deserialize as either a string or an array
    desc::from_tree([](char(& v )[n], const ayu::Tree& tree){
        if (tree.form == ayu::STRING) {
            auto s = uni::OldStr(tree);
            if (s.size() != n) {
                throw ayu::X<ayu::WrongLength>(&v, n, n, s.size());
            }
            for (uint i = 0; i < n; i++) {
                v[i] = s[i];
            }
        }
        else if (tree.form == ayu::ARRAY) {
            auto a = ayu::TreeArraySlice(tree);
            if (a.size() != n) {
                throw ayu::X<ayu::WrongLength>(&v, n, n, a.size());
            }
            for (uint i = 0; i < n; i++) {
                v[i] = char(a[i]);
            }
        }
        else throw ayu::X<ayu::InvalidForm>(ayu::current_location(), tree);
    }),
     // Allow accessing individual elements like an array
    desc::length(desc::template constant<uni::usize>(n)),
    desc::elem_func([](char(& v )[n], uni::usize i){
        if (i < n) return ayu::Reference(&v[i]);
        else return ayu::Reference();
    })
)

 // std::array
AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T, uni::usize n),
    AYU_DESCRIBE_TEMPLATE_TYPE(std::array<T, n>),
    desc::name([]{
        using namespace std::literals;
        static std::string r = uni::old_cat(
            "std::array<"sv + ayu::Type::CppType<T>().name(),
            ", "sv, n, '>'
        );
        return uni::OldStr(r);
    }),
    desc::length(desc::template constant<uni::usize>(n)),
    desc::elem_func([](std::array<T, n>& v, uni::usize i){
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
        static std::string r = uni::old_cat(
            "std::pair<"sv, ayu::Type::CppType<A>().name(),
            ", "sv, ayu::Type::CppType<B>().name(), '>'
        );
        return uni::OldStr(r);
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
        static std::string make () {
            return ""s;
        }
    };
    template <class T>
    struct TupleNames<T> {
        static std::string make () {
            return std::string(Type::CppType<T>().name());
        }
    };
    template <class A, class B, class... Ts>
    struct TupleNames<A, B, Ts...> {
        static std::string make () {
            return uni::old_cat(
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
        static std::string r = uni::old_cat(
            "std::tuple<"sv, ayu::in::TupleNames<Ts...>::make(), '>'
        );
        return uni::OldStr(r);
    }),
    ayu::in::TupleElems<Ts...>::make(
        std::index_sequence_for<Ts...>{}
    )
)
