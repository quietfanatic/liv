// This module implements static-layout type descriptions, which are
// generated at compile time and accessed at runtime to determine how to
// construct, destroy, and transform objects to and from trees.  The
// descriptions are mostly declarative; the actual serialization code is in
// serialize.cpp.

#pragma once

#include <cstddef>
#include <cstdlib>
#include <type_traits>
#include <utility>

#include "accessors-internal.h"
#include "../tree.h"

namespace ayu { struct Reference; }

namespace ayu::in {

// The goal of this module is to allow descriptions to be laid out in memory at
// compile time.  Thanks to recent C++ standards, this is quite possible.
// The caveat is that it is not really possible to generate static pointers to
// static objects.  To get around this, we are representing all "pointers" as
// offsets from the beginning of the description object.

///// MEMORY LAYOUT

 // To compare addresses of disparate types, we need to cast them to a common
 // type.  reinterpret_cast is not allowed in constexprs (and in recent gcc
 // versions, neither are C-style casts), but static_cast to a common base
 // type is.
struct ComparableAddress { };
static_assert(sizeof(ComparableAddress) == 1);

 // We could use [[no_unique_address]] but this is more aggressive at optimizing
 // out empty structs.  The size_t parameter is to prevent multiple CatHeads
 // with the same type from conflicting with one another.
template <size_t, class Head>
struct CatHead;
template <size_t i, class Head>
    requires (!std::is_empty_v<Head>)
struct CatHead<i, Head> {
    Head head;
    constexpr CatHead (const Head& h) : head(h) { }
};
template <size_t i, class Head>
    requires (std::is_empty_v<Head>)
struct CatHead<i, Head> {
     // Ideally this gets discarded by the linker?
    static Head head;
    constexpr CatHead (const Head&) { }
};
template <size_t i, class Head>
    requires (std::is_empty_v<Head>)
Head CatHead<i, Head>::head {};

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
};

template <>
struct Cat<> {
    constexpr Cat () { }
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

template <class T>
using Constructor = void(void*);
template <class T>
using Destructor = void(T*);

 // Determine presence of constructors and stuff using a sfinae trick
template <class T>
constexpr Constructor<T>* default_construct_p = null;
template <class T> requires (requires { new (null) T; })
constexpr Constructor<T>* default_construct_p<T>
    = [](void* target){ new (target) T; };

template <class T, class = void>
constexpr Destructor<T>* destroy_p = null;
template <class T> requires (requires (T& v) { v.~T(); })
constexpr Destructor<T>* destroy_p<T> = [](T* v){ v->~T(); };

 // No SFINAE because these are only used if values() is specified, and
 // values() absolutely requires them.
template <class T>
constexpr bool(* compare_p )(const T&, const T&) =
    [](const T& a, const T& b) { return a == b; };

template <class T>
constexpr void(* assign_p )(T&, const T&) =
    [](T& a, const T& b) { a = b; };

///// DESCRIPTION HEADER

struct Description : ComparableAddress {
    const std::type_info* cpp_type = null;
    uint32 cpp_size = 0;
    uint32 cpp_align = 0;
     // TODO: Merge this with the name dcr?
    OldStr name;

     // Do some property calculations ahead of time
    enum Flags {
        PREFER_ARRAY = 1 << 0,
        PREFER_OBJECT = 1 << 1,
        PREFERENCE = PREFER_ARRAY | PREFER_OBJECT,
//        NO_INHERIT_ATTRS = 1 << 2,
//        NO_INHERIT_ELEMS = 1 << 3,
    };
    uint16 flags = 0;

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

template <class T>
struct DescriptionFor : Description {
    Constructor<T>* default_construct = default_construct_p<T>;
    Destructor<T>* destroy = destroy_p<T>;
};

///// DESCRIPTORS

template <class T>
struct Descriptor : ComparableAddress { };
template <class T>
struct AttachedDescriptor : Descriptor<T> {
    constexpr uint16 get_offset (DescriptionFor<T>& header) {
        return static_cast<ComparableAddress*>(this)
             - static_cast<ComparableAddress*>(&header);
    }
     // Emit this into the static data
    template <class Self>
    static constexpr Self make_static (const Self& self) { return self; }
};
template <class T>
struct DetachedDescriptor : Descriptor<T> {
     // Omit this from the static data
    template <class Self>
    static constexpr ComparableAddress make_static (const Self&) { return {}; }
};

template <class T>
struct NameDcr : AttachedDescriptor<T> {
    OldStr(* f )();
};

template <class T>
struct ToTreeDcr : AttachedDescriptor<T> {
    Tree(* f )(const T&);
};

template <class T>
struct FromTreeDcr : AttachedDescriptor<T> {
    void(* f )(T&, const Tree&);
};

template <class T>
struct SwizzleDcr : AttachedDescriptor<T> {
    void(* f )(T&, const Tree&);
};

template <class T>
struct InitDcr : AttachedDescriptor<T> {
    void(* f )(T&);
};

enum ValueForm {
    VFNULL,
    VFBOOL,
    VFINT64,
    VFDOUBLE,
    VFSTR
};

template <class T>
struct ValueDcr : ComparableAddress {
    uint8 form;
    bool pointer;
     // For optimization (skips a jump table)
    uint8 value_offset;
};

template <class T, class VF, bool pointer>
struct ValueDcrWith;

 // Disable "offsetof within non-standard-layout class is conditionally
 // supported" warning
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"

template <class T, class VF>
struct ValueDcrWith<T, VF, false> : ValueDcr<T> {
     // Don't reorder these
    alignas(void*) VF name;
    alignas(void*) T value;
    constexpr ValueDcrWith (uint8 f, VF n, const T& v) :
        ValueDcr<T>{{}, f, false, offsetof(ValueDcrWith, value)},
        name(n),
        value(v)
    { }
};

template <class T, class VF>
struct ValueDcrWith<T, VF, true> : ValueDcr<T> {
    alignas(void*) VF name;
    const T* ptr;
    constexpr ValueDcrWith (uint8 f, VF n, const T* p) :
        ValueDcr<T>{{}, f, true, offsetof(ValueDcrWith, ptr)},
        name(n),
        ptr(p)
    { }
};

#pragma GCC diagnostic pop

template <class T>
struct ValuesDcr : AttachedDescriptor<T> {
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
            offsets[i] = static_cast<const ComparableAddress*>(
                values.template get<ValueDcr<T>>(i)
            ) - static_cast<const ComparableAddress*>(this);
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
            offsets[i] = static_cast<const ComparableAddress*>(
                values.template get<ValueDcr<T>>(i)
            ) - static_cast<const ComparableAddress*>(this);
        }
    }
};

template <class T>
struct AttrDcr : ComparableAddress {
    OldStr key;
};
template <class T, class Acr>
struct AttrDcrWith : AttrDcr<T> {
    Acr acr;
    constexpr AttrDcrWith (OldStr k, const Acr& a) :
        AttrDcr<T>{{}, k},
        acr(constexpr_acr(a))
    { }
};

template <class T>
struct AttrsDcr : AttachedDescriptor<T> {
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
            offsets[i] = static_cast<ComparableAddress*>(
                attrs.template get<AttrDcr<T>>(i)
            ) - static_cast<ComparableAddress*>(this);
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
struct ElemsDcr : AttachedDescriptor<T> {
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
            offsets[i] = static_cast<const ComparableAddress*>(
                elems.template get<ElemDcr<T>>(i)
            ) - static_cast<const ComparableAddress*>(this);
        }
    }
};

template <class T>
struct KeysDcr : AttachedDescriptor<T> { };
template <class T, class Acr>
struct KeysDcrWith : KeysDcr<T> {
    static_assert(std::is_same_v<typename Acr::AccessorFromType, T>);
    Acr acr;
    constexpr KeysDcrWith (const Acr& a) :
        acr(constexpr_acr(a))
    { }
};

template <class T>
struct AttrFuncDcr : AttachedDescriptor<T> {
    Reference(* f )(T&, OldStr);
};

template <class T>
struct LengthDcr : AttachedDescriptor<T> { };
template <class T, class Acr>
struct LengthDcrWith : LengthDcr<T> {
    static_assert(std::is_same_v<typename Acr::AccessorFromType, T>);
     // TODO: allow other integer-like types here
    static_assert(std::is_same_v<typename Acr::AccessorToType, usize>);
    Acr acr;
    constexpr LengthDcrWith (const Acr& a) :
        acr(constexpr_acr(a))
    { }
};

template <class T>
struct ElemFuncDcr : AttachedDescriptor<T> {
    Reference(* f )(T&, size_t);
};

template <class T>
struct DelegateDcr : AttachedDescriptor<T> { };
template <class T, class Acr>
struct DelegateDcrWith : DelegateDcr<T> {
    static_assert(std::is_same_v<typename Acr::AccessorFromType, T>);
    Acr acr;
    constexpr DelegateDcrWith (const Acr& a) :
        acr(constexpr_acr(a))
    { }
};

template <class T>
struct DefaultConstructDcr : DetachedDescriptor<T> {
    void(* f )(void*);
};
template <class T>
struct DestroyDcr : DetachedDescriptor<T> {
    void(* f )(T*);
};

///// MAKING DESCRIPTIONS

template <class F>
constexpr void for_variadic (F) { }
template <class F, class Arg, class... Args>
constexpr void for_variadic (F f, Arg arg, Args... args) {
    f(arg);
    for_variadic(f, args...);
}

template <class T, class... Dcrs>
using FullDescription = Cat<
    DescriptionFor<T>,
    decltype(Dcrs::make_static(std::declval<Dcrs>()))...
>;

 // Jank compile-time error messages
template <class T>
static void duplicate_descriptors_in_AYU_DESCRIBE () { }
static void element_in_AYU_DESCRIBE_is_not_a_descriptor_for_this_type () { }
static void attrs_cannot_be_combined_with_keys_and_attr_func_in_AYU_DESCRIBE () { }
static void keys_and_attr_func_must_be_together_in_AYU_DESCRIBE () { }
static void elems_cannot_be_combined_with_length_and_elem_func_in_AYU_DESCRIBE () { }
static void length_and_elem_func_must_be_together_in_AYU_DESCRIBE () { }

template <class T, class... Dcrs>
constexpr FullDescription<T, Dcrs...> make_description (OldStr name, const Dcrs&... dcrs) {
    using Desc = FullDescription<T, Dcrs...>;

    static_assert(
        sizeof(T) <= uint32(-1),
        "Cannot describe type larger the 4GB"
    );
    static_assert(
        sizeof(Desc) < 65536,
        "AYU_DESCRIBE description is too large (>64k)"
    );

    Desc desc (
        DescriptionFor<T>{},
        Dcrs::make_static(dcrs)...
    );
    auto& header = *desc.template get<DescriptionFor<T>>(0);
    header.cpp_type = &typeid(T);
    header.cpp_size = sizeof(T);
    header.cpp_align = alignof(T);
    header.name = name;

    for_variadic([&]<class Dcr>(const Dcr& dcr){
        if constexpr (std::is_base_of_v<DefaultConstructDcr<T>, Dcr>) {
            if (header.default_construct != default_construct_p<T>) {
                duplicate_descriptors_in_AYU_DESCRIBE<Dcr>();
            }
            header.default_construct = dcr.f;
        }
        else if constexpr (std::is_base_of_v<DestroyDcr<T>, Dcr>) {
            if (header.destroy != destroy_p<T>) {
                duplicate_descriptors_in_AYU_DESCRIBE<Dcr>();
            }
            header.destroy = dcr.f;
        }
        else if constexpr (std::is_base_of_v<NameDcr<T>, Dcr>) {
#define AYU_APPLY_OFFSET(dcr_type, dcr_name) \
            if (header.dcr_name##_offset) { \
                duplicate_descriptors_in_AYU_DESCRIBE<Dcr>(); \
            } \
            header.dcr_name##_offset = \
                desc.template get<dcr_type<T>>(0)->get_offset(header);
            AYU_APPLY_OFFSET(NameDcr, name)
        }
        else if constexpr (std::is_base_of_v<ToTreeDcr<T>, Dcr>) {
            AYU_APPLY_OFFSET(ToTreeDcr, to_tree)
        }
        else if constexpr (std::is_base_of_v<FromTreeDcr<T>, Dcr>) {
            AYU_APPLY_OFFSET(FromTreeDcr, from_tree)
        }
        else if constexpr (std::is_base_of_v<SwizzleDcr<T>, Dcr>) {
            AYU_APPLY_OFFSET(SwizzleDcr, swizzle)
        }
        else if constexpr (std::is_base_of_v<InitDcr<T>, Dcr>) {
            AYU_APPLY_OFFSET(InitDcr, init)
        }
        else if constexpr (std::is_base_of_v<ValuesDcr<T>, Dcr>) {
            AYU_APPLY_OFFSET(ValuesDcr, values)
        }
        else if constexpr (std::is_base_of_v<AttrsDcr<T>, Dcr>) {
            AYU_APPLY_OFFSET(AttrsDcr, attrs)
            if (!(header.flags & Description::PREFERENCE)) {
                header.flags |= Description::PREFER_OBJECT;
            }
        }
        else if constexpr (std::is_base_of_v<KeysDcr<T>, Dcr>) {
            AYU_APPLY_OFFSET(KeysDcr, keys)
            if (!(header.flags & Description::PREFERENCE)) {
                header.flags |= Description::PREFER_OBJECT;
            }
        }
        else if constexpr (std::is_base_of_v<AttrFuncDcr<T>, Dcr>) {
            AYU_APPLY_OFFSET(AttrFuncDcr, attr_func)
            if (!(header.flags & Description::PREFERENCE)) {
                header.flags |= Description::PREFER_OBJECT;
            }
        }
        else if constexpr (std::is_base_of_v<ElemsDcr<T>, Dcr>) {
            AYU_APPLY_OFFSET(ElemsDcr, elems)
            if (!(header.flags & Description::PREFERENCE)) {
                header.flags |= Description::PREFER_ARRAY;
            }
        }
        else if constexpr (std::is_base_of_v<LengthDcr<T>, Dcr>) {
            AYU_APPLY_OFFSET(LengthDcr, length)
            if (!(header.flags & Description::PREFERENCE)) {
                header.flags |= Description::PREFER_ARRAY;
            }
        }
        else if constexpr (std::is_base_of_v<ElemFuncDcr<T>, Dcr>) {
            AYU_APPLY_OFFSET(ElemFuncDcr, elem_func)
            if (!(header.flags & Description::PREFERENCE)) {
                header.flags |= Description::PREFER_ARRAY;
            }
        }
        else if constexpr (std::is_base_of_v<DelegateDcr<T>, Dcr>) {
            AYU_APPLY_OFFSET(DelegateDcr, delegate)
        }
#undef AYU_APPLY_OFFSET
        else {
            element_in_AYU_DESCRIBE_is_not_a_descriptor_for_this_type();
        }
    }, dcrs...);
    if (header.attrs_offset &&
        (header.keys_offset || header.attr_func_offset)
    ) {
        attrs_cannot_be_combined_with_keys_and_attr_func_in_AYU_DESCRIBE();
    }
    if ((header.keys_offset && !header.attr_func_offset) ||
        (header.attr_func_offset && !header.keys_offset)
    ) {
        keys_and_attr_func_must_be_together_in_AYU_DESCRIBE();
    }
    if (header.elems_offset &&
        (header.length_offset || header.elem_func_offset)
    ) {
        elems_cannot_be_combined_with_length_and_elem_func_in_AYU_DESCRIBE();
    }
    if ((header.length_offset && !header.elem_func_offset) ||
        (header.elem_func_offset && !header.length_offset)
    ) {
        length_and_elem_func_must_be_together_in_AYU_DESCRIBE();
    }

    return desc;
}

} // namespace ayu::in
