// This module implements static-layout haccable descriptions, which are
// generated at compile time and accessed at runtime to determine how to
// transform objects to and from trees.  The descriptions are mostly
// declarative; the actual serialization code is in serialize.cpp.

#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "accessors.h"
#include "tree.h"

namespace hacc { struct Reference; }

namespace hacc::in {

// The goal of this module is to allow descriptions to be laid out in memory
//  at compile time.  Thanks to recent C++ standards, this is quite possible.
//  The caveat is that it is not really possible to generate static pointers
//  to static objects.  To get around this, we are representing all "pointers"
//  as offsets from the beginning of the description object.

///// MEMORY LAYOUT

 // To compare addresses of disparate types, we need to cast them to a common
 //  type.  reinterpret_cast is not allowed in constexprs (and in recent gcc
 //  versions, neither are C-style casts), but static_cast to a common base
 //  type is.
struct ComparableAddress { };
static_assert(sizeof(ComparableAddress) == 1);

 // [[no_unique_address]] causes internal compiler errors in my current version
 //  of GCC (9.1), so we're still relying on this workaround.
 // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98463
template <size_t, class Head, bool = std::is_empty_v<Head>>
struct CatHead;
template <size_t i, class Head>
struct CatHead<i, Head, false> {
    Head head;
    constexpr CatHead (const Head& h) : head(h) { }
};
template <size_t i, class Head>
struct CatHead<i, Head, true> {
     // Ideally this gets discarded by the linker?
    static Head head;
    constexpr CatHead (const Head&) { }
};
template <size_t i, class Head>
Head CatHead<i, Head, true>::head {};

template <class...>
struct Cat;

template <class Head, class... Tail>
struct Cat<Head, Tail...> : CatHead<sizeof...(Tail), Head>, Cat<Tail...> {
    constexpr Cat (const Head& h, const Tail&... t) :
        CatHead<sizeof...(Tail), Head>(h), Cat<Tail...>(t...)
    { }

    template <class T>
    constexpr T* get (uint16 n) {
        if constexpr (std::is_base_of<T, Head>::value) {
            if (n == 0) return &this->CatHead<sizeof...(Tail), Head>::head;
            else return Cat<Tail...>::template get<T>(n-1);
        }
        else return Cat<Tail...>::template get<T>(n);
    }
    template <class T>
    constexpr const T* get (uint16 n) const {
        if constexpr (std::is_base_of<T, Head>::value) {
            if (n == 0) return &this->CatHead<sizeof...(Tail), Head>::head;
            else return Cat<Tail...>::template get<T>(n-1);
        }
        else return Cat<Tail...>::template get<T>(n);
    }

    template <class T>
    static constexpr uint16 count () {
        return Cat<Tail...>::template count<T>() + (std::is_base_of<T, Head>::value ? 1 : 0);
    }
};

template <>
struct Cat<> {
    constexpr Cat () { }

    template <class T>
    static constexpr uint16 count () {
        return 0;
    }
    template <class T>
    constexpr T* get (uint16) {
        return null;
    }
    template <class T>
    constexpr const T* get (uint16) const {
        return null;
    }
};

///// CPP TYPE TRAITS

using DefaultConstructor = void(void*);
using Destructor = void(Mu&);

 // Determine presence of constructors and stuff using a sfinae trick
template <class T, class = void>
static constexpr DefaultConstructor* default_construct_p = null;
template <class T>
static constexpr DefaultConstructor* default_construct_p<
    T, std::void_t<decltype(new (null) T)>
> = [](void* target){ new (target) T; };

template <class T, class = void>
static constexpr Destructor* destruct_p = null;
template <class T>
static constexpr Destructor* destruct_p<
    T, std::void_t<decltype(std::declval<T>().~T())>
> = [](Mu& v){ reinterpret_cast<T&>(v).~T(); };

 // No SFINAE because these are only used if values() is specified, and
 //  values() absolutely requires them.
template <class T>
static constexpr bool(* compare_p )(const T&, const T&) =
[](const T& a, const T& b) { return a == b; };

template <class T>
static constexpr void(* assign_p )(T&, const T&) =
[](T& a, const T& b) { a = b; };

///// DESCRIPTORS

template <class T>
struct Descriptor : ComparableAddress { };

template <class T>
struct NameDcr : Descriptor<T> {
    Str(* f )();
};

template <class T>
struct ToTreeDcr : Descriptor<T> {
    Tree(* f )(const T&);
};

template <class T>
struct FromTreeDcr : Descriptor<T> {
    void(* f )(T&, const Tree&);
};

template <class T>
struct SwizzleDcr : Descriptor<T> {
    void(* f )(T&, const Tree&);
};

template <class T>
struct InitDcr : Descriptor<T> {
    void(* f )(T&);
};

enum ValueForm {
    VFNULL,
    VFBOOL,
    VFINT64,
    VFDOUBLE,
    VFCONSTCHARP
};

template <class T>
struct ValueDcr : ComparableAddress {
    uint8 form;
    bool pointer;
};

template <class T, class VF, bool pointer>
struct ValueDcrWith;

template <class T, class VF>
struct ValueDcrWith<T, VF, false> : ValueDcr<T> {
    VF name;
    alignas(void*) T value;
    constexpr ValueDcrWith (uint8 f, VF n, const T& v) :
        ValueDcr<T>{{}, f, false},
        name(n),
        value(v)
    { }
};

template <class T, class VF>
struct ValueDcrWith<T, VF, true> : ValueDcr<T> {
    VF name;
    const T* ptr;
    constexpr ValueDcrWith (uint8 f, VF n, const T* p) :
        ValueDcr<T>{{}, f, true},
        name(n),
        ptr(p)
    { }
};

template <class T>
struct ValuesDcr : Descriptor<T> {
    bool(* compare )(const T&, const T&);
    void(* assign )(T&, const T&);
    uint16 n_values;
};
template <class T, class... Values>
struct ValuesDcrWith : ValuesDcr<T> {
    uint16 offsets [sizeof...(Values)] {};
    Cat<Values...> values;
    constexpr ValuesDcrWith (const Values&... vs) :
        ValuesDcr<T>{{}, compare_p<T>, assign_p<T>, sizeof...(Values)},
        values(vs...)
    {
        for (uint i = 0; i < sizeof...(Values); i++) {
            offsets[i] = static_cast<const ComparableAddress*>(values.template get<ValueDcr<T>>(i))
                       - static_cast<const ComparableAddress*>(this);
        }
    }
    constexpr ValuesDcrWith (
        bool(* compare )(const T&, const T&),
        void(* assign )(T&, const T&),
        const Values&... vs
    ) :
        ValuesDcr<T>{{}, compare, assign, sizeof...(Values)},
        values(vs...)
    {
        for (uint i = 0; i < sizeof...(Values); i++) {
            offsets[i] = static_cast<const ComparableAddress*>(values.template get<ValueDcr<T>>(i))
                       - static_cast<const ComparableAddress*>(this);
        }
    }
};

template <class T>
struct AttrDcr : ComparableAddress {
    Str key;
};
template <class T, class Acr>
struct AttrDcrWith : AttrDcr<T> {
    Acr acr;
    constexpr AttrDcrWith (Str k, const Acr& a) :
        AttrDcr<T>{{}, k},
        acr(constexpr_acr(a))
    { }
};

template <class T>
struct AttrsDcr : Descriptor<T> {
    uint16 n_attrs;
};

template <class T, class... Attrs>
struct AttrsDcrWith : AttrsDcr<T> {
    uint16 offsets [sizeof...(Attrs)] {};
    Cat<Attrs...> attrs;
    constexpr AttrsDcrWith (const Attrs&... as) :
        AttrsDcr<T>{{}, uint16(sizeof...(Attrs))},
        attrs(as...)
    {
        for (uint i = 0; i < sizeof...(Attrs); i++) {
            offsets[i] = static_cast<ComparableAddress*>(attrs.template get<AttrDcr<T>>(i))
                       - static_cast<ComparableAddress*>(this);
        }
    }
};

template <class T>
struct ElemDcr : ComparableAddress { };
template <class T, class Acr>
struct ElemDcrWith : ElemDcr<T> {
    Acr acr;
    constexpr ElemDcrWith (const Acr& a) :
        acr(constexpr_acr(a))
    { }
};

template <class T>
struct ElemsDcr : Descriptor<T> {
    uint16 n_elems;
};

template <class T, class... Elems>
struct ElemsDcrWith : ElemsDcr<T> {
    uint16 offsets [sizeof...(Elems)] {};
    Cat<Elems...> elems;
    constexpr ElemsDcrWith (const Elems&... es) :
        ElemsDcr<T>{{}, uint16(sizeof...(Elems))},
        elems(es...)
    {
        for (uint i = 0; i < sizeof...(Elems); i++) {
            offsets[i] = static_cast<const ComparableAddress*>(elems.template get<ElemDcr<T>>(i))
                       - static_cast<const ComparableAddress*>(this);
        }
    }
};

template <class T>
struct KeysDcr : Descriptor<T> { };
template <class T, class Acr>
struct KeysDcrWith : KeysDcr<T> {
    static_assert(std::is_same_v<typename Acr::AccessorFromType, T>);
    static_assert(std::is_same_v<typename Acr::AccessorToType, std::vector<String>>);
    Acr acr;
    constexpr KeysDcrWith (const Acr& a) :
        acr(constexpr_acr(a))
    { }
};

template <class T>
struct AttrFuncDcr : Descriptor<T> {
    Reference(* f )(T&, Str);
};

template <class T>
struct LengthDcr : Descriptor<T> { };
template <class T, class Acr>
struct LengthDcrWith : LengthDcr<T> {
    static_assert(std::is_same_v<typename Acr::AccessorFromType, T>);
    static_assert(std::is_same_v<typename Acr::AccessorToType, usize>);
    Acr acr;
    constexpr LengthDcrWith (const Acr& a) :
        acr(constexpr_acr(a))
    { }
};

template <class T>
struct ElemFuncDcr : Descriptor<T> {
    Reference(* f )(T&, size_t);
};

template <class T>
struct DelegateDcr : Descriptor<T> {
};
template <class T, class Acr>
struct DelegateDcrWith : DelegateDcr<T> {
    static_assert(std::is_same_v<typename Acr::AccessorFromType, T>);
    Acr acr;
    constexpr DelegateDcrWith (const Acr& a) :
        acr(constexpr_acr(a))
    { }
};

///// DESCRIPTION HEADER

struct Description : ComparableAddress {
    const std::type_info* cpp_type = null;
    size_t cpp_size = 0;
    DefaultConstructor* default_construct = null;
    Destructor* destruct = null;

    Str name;

    uint16 name_offset = 0;
    uint16 to_tree_offset = 0;
    uint16 from_tree_offset = 0;
    uint16 swizzle_offset = 0;
    uint16 init_offset = 0;
    uint16 values_offset = 0;
    uint16 attrs_offset = 0;
    uint16 elems_offset = 0;
    uint16 keys_offset = 0;
    uint16 attr_func_offset = 0;
    uint16 length_offset = 0;
    uint16 elem_func_offset = 0;
    uint16 delegate_offset = 0;
};

///// MAKING DESCRIPTIONS

template <class T, class...>
struct AssertAllDcrs;
template <class T, class Head, class... Tail>
struct AssertAllDcrs<T, Head, Tail...> {
    static_assert(std::is_base_of_v<Descriptor<T>, Head>, "Element in HACCABLE description is not a descriptor for this type");
};
template <class T>
struct AssertAllDcrs<T> { };

template <class T, class... Dcrs>
using FullDescription = Cat<
    Description,
    Dcrs...
>;

template <class T, class... Dcrs>
constexpr FullDescription<T, Dcrs...> make_description (Str name, const Dcrs&... dcrs) {
    AssertAllDcrs<T, Dcrs...>{};
    using Desc = FullDescription<T, Dcrs...>;
    static_assert(sizeof(Desc) < 65536, "Haccable description is too large (>64k)");

    Desc desc (
        Description{},
        dcrs...
    );

    auto& header = *desc.template get<Description>(0);
    header.cpp_type = &typeid(T);
    header.cpp_size = sizeof(T);
     // My stdlib is missing aligned_alloc so it's easier to not deal with alignment
    static_assert(alignof(T) <= alignof(std::max_align_t), "Haccability of types with larger than standard alignment is not currently supported, sorry.");
    header.default_construct = default_construct_p<T>;
    header.destruct = destruct_p<T>;
    header.name = name;

     // template lambdas please?
#define APPLY_OFFSET(dcr_name, dcr_type) \
    { \
        constexpr uint16 count = Desc::template count<dcr_type<T>>(); \
        static_assert(count <= 1, "Multiple " #dcr_name " descriptors in haccable description"); \
        if constexpr (count) { \
            header.dcr_name##_offset = \
                static_cast<ComparableAddress*>(desc.template get<dcr_type<T>>(0)) \
              - static_cast<ComparableAddress*>(&header); \
        } \
    }
    APPLY_OFFSET(name, NameDcr)
    APPLY_OFFSET(to_tree, ToTreeDcr)
    APPLY_OFFSET(from_tree, FromTreeDcr)
    APPLY_OFFSET(swizzle, SwizzleDcr)
    APPLY_OFFSET(init, InitDcr)
    APPLY_OFFSET(values, ValuesDcr)
    APPLY_OFFSET(attrs, AttrsDcr)
    APPLY_OFFSET(elems, ElemsDcr)
    APPLY_OFFSET(keys, KeysDcr)
    APPLY_OFFSET(attr_func, AttrFuncDcr)
    APPLY_OFFSET(length, LengthDcr)
    APPLY_OFFSET(elem_func, ElemFuncDcr)
    APPLY_OFFSET(delegate, DelegateDcr)

    return desc;
}

} // namespace hacc::in
