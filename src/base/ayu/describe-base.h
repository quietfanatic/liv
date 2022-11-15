 // This is the interface for describing types to ayu.
 // TODO: add documentation for all the descriptors.

#pragma once

#include <type_traits>
#include <typeinfo>

#include "internal/accessors-internal.h"
#include "internal/descriptors-internal.h"
#include "common.h"
#include "reference.h"
#include "tree.h"

namespace ayu {

template <class T, bool has_members = std::is_class_v<T> || std::is_union_v<T>>
struct _AYU_DescribeBase;

template <class T>
struct _AYU_DescribeBase<T, false> {
    static constexpr auto name (Str(* f )());
    static constexpr auto to_tree (Tree(* f )(const T&));
    static constexpr auto from_tree (void(* f )(T&, const Tree&));
    static constexpr auto swizzle (void(* f )(T&, const Tree&));
    static constexpr auto init (void(* f )(T&));

    template <class... Values>
    static constexpr auto values (const Values&... vs);
    template <class... Values>
    static constexpr auto values_custom (
        bool(* compare )(const T&, const T&),
        void(* assign )(T&, const T&),
        const Values&... vs
    );
    template <class N>
    static constexpr auto value (const N& n, const T& v);
    template <class N>
    static constexpr auto value_pointer (const N& n, const T* v);

    template <class... Attrs>
    static constexpr auto attrs (const Attrs&... as);
    template <class Acr>
    static constexpr auto attr (
        Str key,
        const Acr& acr,
        in::AttrFlags flags = in::AttrFlags(0)
    );
    template <class... Elems>
    static constexpr auto elems (const Elems&... es);
    template <class Acr>
    static constexpr auto elem (
        const Acr& acr,
        in::AttrFlags flags = in::AttrFlags(0)
    );
    template <class Acr>
    static constexpr auto keys (const Acr& acr);
    static constexpr auto attr_func (Reference(* f )(T&, Str));
    template <class Acr>
    static constexpr auto length (const Acr& acr);
    static constexpr auto elem_func (Reference(* f )(T&, usize));
    template <class Acr>
    static constexpr auto delegate (const Acr& acr);

    template <class B>
    static constexpr auto base (
        in::AccessorFlags flags = in::AccessorFlags(0)
    );
    template <class M>
    static constexpr auto ref_func (
        M&(* f )(T&),
        in::AccessorFlags flags = in::AccessorFlags(0)
    );
    template <class M>
    static constexpr auto const_ref_func (
        const M&(* f )(const T&),
        in::AccessorFlags flags = in::AccessorFlags(0)
    );
    template <class M>
    static constexpr auto const_ref_funcs (
        const M&(* g )(const T&),
        void(* s )(T&, const M&),
        in::AccessorFlags flags = in::AccessorFlags(0)
    );
    template <class M>
    static constexpr auto value_func (
        M(* f )(const T&),
        in::AccessorFlags flags = in::AccessorFlags(0)
    );
    template <class M>
    static constexpr auto value_funcs (
        M(* g )(const T&),
        void(* s )(T&, M),
        in::AccessorFlags flags = in::AccessorFlags(0)
    );
    template <class M>
    static constexpr auto mixed_funcs (
        M(* g )(const T&),
        void(* s )(T&, const M&),
        in::AccessorFlags flags = in::AccessorFlags(0)
    );

    template <class M>
    static constexpr auto assignable (
        in::AccessorFlags flags = in::AccessorFlags(0)
    );

     // This one is not constexpr, so it is only valid in attr_func, elem_func,
     // or reference_func.
    template <class M>
    static auto variable (
        M&& v,
        in::AccessorFlags flags = in::AccessorFlags(0)
    );

    template <class M>
    static constexpr auto constant (
        const M& v,
        in::AccessorFlags flags = in::AccessorFlags(0)
    );
    template <class M>
    static constexpr auto constant_pointer (
        const M* p,
        in::AccessorFlags flags = in::AccessorFlags(0)
    );
    static constexpr auto reference_func (
        Reference(* f )(T&),
        in::AccessorFlags flags = in::AccessorFlags(0)
    );

    static constexpr in::AttrFlags optional = in::ATTR_OPTIONAL;
     // NYI for array-like types
    static constexpr in::AttrFlags inherit = in::ATTR_INHERIT;

    static constexpr in::AccessorFlags readonly = in::ACR_READONLY;
    static constexpr in::AccessorFlags anchored_to_parent = in::ACR_ANCHORED_TO_PARENT;
     // Internal
    template <class... Dcrs>
    static constexpr auto _ayu_describe (
        ayu::Str name, const Dcrs&... dcrs
    );
};

 // This contains functions that aren't valid for scalar types like int
template <class T>
struct _AYU_DescribeBase<T, true> : _AYU_DescribeBase<T, false> {
    template <class T2, class M>
    static constexpr auto member (
        M T2::* mp,
        in::AccessorFlags flags = in::AccessorFlags(0)
    );
    template <class T2, class M>
    static constexpr auto const_member (
        const M T2::* mp,
        in::AccessorFlags flags = in::AccessorFlags(0)
    );
};

} // namespace ayu

#include "internal/describe-base-internal.h"
