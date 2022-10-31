#pragma once

 // This is the interface for describing types to ayu.
 // TODO: add documentation for all the facets.

#include "accessors.h"
#include "common.h"
#include "description.h"
#include "registry.h"
#include "reference.h"
#include "tree.h"

#include <type_traits>
#include <typeinfo>

namespace ayu {

template <class T, bool has_members = std::is_class_v<T> || std::is_union_v<T>>
struct DescribeBase;

template <class T>
struct DescribeBase<T, false> {
    template <class... Dcrs>
    static constexpr in::FullDescription<T, Dcrs...> describe (
        ayu::Str name, const Dcrs&... dcrs
    ) {
        return in::make_description<T, Dcrs...>(name, dcrs...);
    }

    static constexpr in::NameDcr<T> name (Str(* f )()) {
        return in::NameDcr<T>{{}, f};
    }

    static constexpr in::ToTreeDcr<T> to_tree (Tree(* f )(const T&)) {
        return in::ToTreeDcr<T>{{}, f};
    }
    static constexpr in::FromTreeDcr<T> from_tree (void(* f )(T&, const Tree&)) {
        return in::FromTreeDcr<T>{{}, f};
    }
    static constexpr in::SwizzleDcr<T> swizzle (void(* f )(T&, const Tree&)) {
        return in::SwizzleDcr<T>{{}, f};
    }
    static constexpr in::InitDcr<T> init (void(* f )(T&)) {
        return in::InitDcr<T>{{}, f};
    }

    template <class... Values>
    static constexpr in::ValuesDcrWith<T, Values...> values (const Values&... vs) {
        return in::ValuesDcrWith<T, Values...>(vs...);
    }
    template <class... Values>
    static constexpr in::ValuesDcrWith<T, Values...> values_custom (
        bool(* compare )(const T&, const T&),
        void(* assign )(T&, const T&),
        const Values&... vs
    ) {
        return in::ValuesDcrWith<T, Values...>(compare, assign, vs...);
    }
    template <class N>
    static constexpr auto value (const N& n, const T& v) {
        if constexpr (std::is_null_pointer_v<N>) {
            return in::ValueDcrWith<T, Null, false>(in::VFNULL, n, v);
        }
        else if constexpr (std::is_same_v<N, bool>) {
            return in::ValueDcrWith<T, bool, false>(in::VFBOOL, n, v);
        }
        else if constexpr (std::is_integral_v<N>) {
            return in::ValueDcrWith<T, int64, false>(in::VFINT64, n, v);
        }
        else if constexpr (std::is_floating_point_v<N>) {
            return in::ValueDcrWith<T, double, false>(in::VFDOUBLE, n, v);
        }
        else {
             // Assume const char* for simplicity
             // TODO: do we want this to be Str?
            return in::ValueDcrWith<T, const char*, false>(in::VFCONSTCHARP, n, v);
        }
    }
    template <class N>
    static constexpr auto value_pointer (const N& n, const T* v) {
        if constexpr (std::is_null_pointer_v<N>) {
            return in::ValueDcrWith<T, Null, true>(in::VFNULL, n, v);
        }
        else if constexpr (std::is_same_v<N, bool>) {
            return in::ValueDcrWith<T, bool, true>(in::VFBOOL, n, v);
        }
        else if constexpr (std::is_integral_v<N>) {
            return in::ValueDcrWith<T, int64, true>(in::VFINT64, n, v);
        }
        else if constexpr (std::is_floating_point_v<N>) {
            return in::ValueDcrWith<T, double, true>(in::VFDOUBLE, n, v);
        }
        else {
             // Assume const char* for simplicity
            return in::ValueDcrWith<T, const char*, true>(in::VFCONSTCHARP, n, v);
        }
    }

    template <class... Attrs>
    static constexpr in::AttrsDcrWith<T, Attrs...> attrs (const Attrs&... as) {
        return in::AttrsDcrWith<T, Attrs...>(as...);
    }
    template <class Acr>
    static constexpr auto attr (
        Str key,
        const Acr& acr,
        in::AttrFlags flags = in::AttrFlags(0)
    ) {
         // Implicit member().
        if constexpr (std::is_member_object_pointer_v<Acr>) {
            return attr(key, DescribeBase<T, true>::member(acr), flags);
        }
        else {
            static_assert(
                std::is_same_v<typename Acr::AccessorFromType, T>,
                "Second argument to attr() is not an accessor of this type"
            );
            auto r = in::AttrDcrWith<T, Acr>(key, acr);
            r.acr.attr_flags = flags;
            return r;
        }
    }
    template <class... Elems>
    static constexpr in::ElemsDcrWith<T, Elems...> elems (const Elems&... es) {
        return in::ElemsDcrWith<T, Elems...>(es...);
    }
    template <class Acr>
    static constexpr auto elem (
        const Acr& acr,
        in::AttrFlags flags = in::AttrFlags(0)
    ) {
        if constexpr (std::is_member_object_pointer_v<Acr>) {
            return elem(DescribeBase<T, true>::member(acr), flags);
        }
        else {
            static_assert(
                std::is_same_v<typename Acr::AccessorFromType, T>,
                "First argument to elem() is not an accessor of this type"
            );
            auto r = in::ElemDcrWith<T, Acr>(acr);
            r.acr.attr_flags = flags;
            return r;
        }
    }
    template <class Acr>
    static constexpr in::KeysDcrWith<T, Acr> keys (const Acr& acr) {
        return in::KeysDcrWith<T, Acr>(acr);
    }
    static constexpr in::AttrFuncDcr<T> attr_func (Reference(* f )(T&, Str)) {
        return in::AttrFuncDcr<T>{{}, f};
    }
    template <class Acr>
    static constexpr in::LengthDcrWith<T, Acr> length (const Acr& acr) {
        return in::LengthDcrWith<T, Acr>(acr);
    }
    static constexpr in::ElemFuncDcr<T> elem_func (Reference(* f )(T&, usize)) {
        return in::ElemFuncDcr<T>{{}, f};
    }
    template <class Acr>
    static constexpr in::DelegateDcrWith<T, Acr> delegate (const Acr& acr) {
        return in::DelegateDcrWith<T, Acr>(acr);
    }

    template <class B>
    static constexpr in::BaseAcr2<T, B> base (
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::BaseAcr2<T, B>(flags);
    }
    template <class M>
    static constexpr in::RefFuncAcr2<T, M> ref_func (
        M&(* f )(T&),
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::RefFuncAcr2<T, M>(f, flags);
    }
    template <class M>
    static constexpr in::ConstRefFuncAcr2<T, M> const_ref_func (
        const M&(* f )(const T&),
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::ConstRefFuncAcr2<T, M>(f, flags);
    }
    template <class M>
    static constexpr in::RefFuncsAcr2<T, M> const_ref_funcs (
        const M&(* g )(const T&),
        void(* s )(T&, const M&),
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::RefFuncsAcr2<T, M>(g, s, flags);
    }
    template <class M>
    static constexpr in::ValueFuncAcr2<T, M> value_func (
        M(* f )(const T&),
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::ValueFuncAcr2<T, M>(f, flags);
    }
    template <class M>
    static constexpr in::ValueFuncsAcr2<T, M> value_funcs (
        M(* g )(const T&),
        void(* s )(T&, M),
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::ValueFuncsAcr2<T, M>(g, s, flags);
    }
    template <class M>
    static constexpr in::MixedFuncsAcr2<T, M> mixed_funcs (
        M(* g )(const T&),
        void(* s )(T&, const M&),
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::MixedFuncsAcr2<T, M>(g, s, flags);
    }

    template <class M>
    static constexpr in::AssignableAcr2<T, M> assignable (
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::AssignableAcr2<T, M>(flags);
    }

     // This one is not constexpr, so it is only valid in attr_func, elem_func,
     // or reference_func.
    template <class M>
    static in::VariableAcr2<T, M> variable (
        M&& v,
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::VariableAcr2<T, M>(std::move(v), flags);
    }

    template <class M>
    static constexpr in::ConstantAcr2<T, M> constant (
        const M& v,
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::ConstantAcr2<T, M>(v, flags);
    }
    template <class M>
    static constexpr in::ConstantPointerAcr2<T, M> constant_pointer (
        const M* p,
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::ConstantPointerAcr2<T, M>(p, flags);
    }
    static constexpr in::ReferenceFuncAcr2<T> reference_func (
        Reference(* f )(T&),
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::ReferenceFuncAcr2<T>(f, flags);
    }

    static constexpr in::AttrFlags optional = in::ATTR_OPTIONAL;
     // NYI
    static constexpr in::AttrFlags inherit = in::ATTR_INHERIT;

    static constexpr in::AccessorFlags readonly = in::ACR_READONLY;
    static constexpr in::AccessorFlags anchored_to_parent = in::ACR_ANCHORED_TO_PARENT;
};

 // This contains functions that aren't valid for scalar types like int
template <class T>
struct DescribeBase<T, true> : DescribeBase<T, false> {
    template <class T2, class M>
    static constexpr in::MemberAcr2<T, M> member (
        M T2::* mp,
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::MemberAcr2<T, M>(mp, flags);
    }
    template <class M>
    static constexpr in::MemberAcr2<T, M> const_member (
        const M T::* mp,
        in::AccessorFlags flags = in::AccessorFlags(0)
    ) {
        return in::MemberAcr2<T, M>(const_cast<M T::*>(mp), flags | in::ACR_READONLY);
    }
};

} // namespace ayu

// TODO: Put _ before these names so they don't accidentally shadow other things

 // Stringify name as early as possible to avoid macro expansion
#define AYU_DESCRIBE_BEGIN(T) AYU_DESCRIBE_BEGIN_NAME(T, #T)
#define AYU_DESCRIBE_BEGIN_NAME(T, name) \
namespace ayu_desc { \
template <> \
struct Describe<T> : ayu::DescribeBase<T> { \
    static constexpr bool defined = true; \
    static constexpr auto full_description = ayu::DescribeBase<T>::describe(name,

#define AYU_DESCRIBE_END(T) \
    ); \
    static const ayu::in::Description* const description; \
}; \
const ayu::in::Description* const Describe<T>::description = \
    ayu::in::register_description( \
        full_description.template get<ayu::in::Description>(0) \
    ); \
}

#define AYU_DESCRIBE(T, ...) AYU_DESCRIBE_NAME(T, #T, __VA_ARGS__)
#define AYU_DESCRIBE_NAME(T, name, ...) \
AYU_DESCRIBE_BEGIN_NAME(T, name) \
    __VA_ARGS__ \
AYU_DESCRIBE_END(T)

 // The only way to make an empty description work
#define AYU_DESCRIBE_0(T) \
namespace ayu_desc { \
template <> \
struct Describe<T> : ayu::DescribeBase<T> { \
    using hcb = ayu::DescribeBase<T>; \
    static constexpr bool defined = true; \
    static constexpr auto full_description = ayu::DescribeBase<T>::describe(#T); \
    static const ayu::in::Description* const description; \
}; \
const ayu::in::Description* const Describe<T>::description = \
    ayu::in::register_description( \
        full_description.template get<ayu::in::Description>(0) \
    ); \
}

#define AYU_DESCRIBE_TEMPLATE_PARAMS(...) <__VA_ARGS__>
#define AYU_DESCRIBE_TEMPLATE_TYPE(...) __VA_ARGS__

#define AYU_DESCRIBE_TEMPLATE_BEGIN(params, T) \
namespace ayu_desc { \
template params \
struct Describe<T> : ayu::DescribeBase<T> { \
    /* annoying lookup problems in templates */ \
    using hcb = ayu::DescribeBase<T>; \
    static constexpr bool defined = true; \
    static constexpr auto full_description = hcb::describe(ayu::Str(),

#define AYU_DESCRIBE_TEMPLATE_END(params, T) \
    ); \
    static const ayu::in::Description* const description; \
}; \
template params \
const ayu::in::Description* const Describe<T>::description = \
    ayu::in::register_description( \
        full_description.template get<ayu::in::Description>(0) \
    ); \
}

#define AYU_DESCRIBE_ESCAPE(...) __VA_ARGS__

#define AYU_DESCRIBE_TEMPLATE(params, T, ...) \
AYU_DESCRIBE_TEMPLATE_BEGIN(AYU_DESCRIBE_ESCAPE(params), AYU_DESCRIBE_ESCAPE(T)) \
    __VA_ARGS__ \
AYU_DESCRIBE_TEMPLATE_END(AYU_DESCRIBE_ESCAPE(params), AYU_DESCRIBE_ESCAPE(T))

#define AYU_DESCRIBE_INSTANTIATE(T) \
static_assert(ayu_desc::Describe<T>::description);
