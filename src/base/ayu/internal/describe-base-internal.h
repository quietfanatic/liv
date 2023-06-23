 // This must ONLY be included by ../describe-base.h
#pragma once

namespace ayu {

template <class T>
constexpr auto _AYU_DescribeBase<T>::name (StaticString(* f )()) {
    return in::NameDcr<T>{{}, f};
}

template <class T>
constexpr auto _AYU_DescribeBase<T>::to_tree (Tree(* f )(const T&)) {
    return in::ToTreeDcr<T>{{}, f};
}
template <class T>
constexpr auto _AYU_DescribeBase<T>::from_tree (void(* f )(T&, const Tree&)) {
    return in::FromTreeDcr<T>{{}, f};
}
template <class T>
constexpr auto _AYU_DescribeBase<T>::swizzle (void(* f )(T&, const Tree&)) {
    return in::SwizzleDcr<T>{{}, f};
}
template <class T>
constexpr auto _AYU_DescribeBase<T>::init (void(* f )(T&)) {
    return in::InitDcr<T>{{}, f};
}

template <class T>
constexpr auto _AYU_DescribeBase<T>::default_construct (void(* f )(void*)) {
    return in::DefaultConstructDcr<T>{{}, f};
}
template <class T>
constexpr auto _AYU_DescribeBase<T>::destroy (void(* f )(T*)) {
    return in::DestroyDcr<T>{{}, f};
}

template <class T>
template <class... Values>
    requires (requires (T v) { v == v; v = v; })
constexpr auto _AYU_DescribeBase<T>::values (const Values&... vs) {
    return in::ValuesDcrWith<T, Values...>(vs...);
}
template <class T>
template <class... Values>
constexpr auto _AYU_DescribeBase<T>::values_custom (
    bool(* compare )(const T&, const T&),
    void(* assign )(T&, const T&),
    const Values&... vs
) {
    return in::ValuesDcrWith<T, Values...>(compare, assign, vs...);
}
template <class T>
template <class N>
    requires (requires (T v) { T(std::move(v)); })
constexpr auto _AYU_DescribeBase<T>::value (const N& n, T&& v) {
    if constexpr (std::is_null_pointer_v<N>) {
        return in::ValueDcrWith<T, Null, false>(in::VFNULL, n, std::move(v));
    }
    else if constexpr (std::is_same_v<N, bool>) {
        return in::ValueDcrWith<T, bool, false>(in::VFBOOL, n, std::move(v));
    }
    else if constexpr (std::is_integral_v<N>) {
        return in::ValueDcrWith<T, int64, false>(in::VFINT64, n, std::move(v));
    }
    else if constexpr (std::is_floating_point_v<N>) {
        return in::ValueDcrWith<T, double, false>(in::VFDOUBLE, n, std::move(v));
    }
    else {
         // Assume something that can be made into a StaticString
        return in::ValueDcrWith<T, StaticString, false>(
            in::VFSTRING, StaticString::Static(n), std::move(v)
        );
    }
}
template <class T>
template <class N>
    requires (requires (const T& v) { T(v); })
constexpr auto _AYU_DescribeBase<T>::value (const N& n, const T& v) {
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
        return in::ValueDcrWith<T, StaticString, false>(
            in::VFSTRING, StaticString::Static(n), v
        );
    }
}
template <class T>
template <class N>
constexpr auto _AYU_DescribeBase<T>::value_pointer (const N& n, const T* v) {
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
        return in::ValueDcrWith<T, StaticString, true>(
            in::VFSTRING, StaticString::Static(n), v
        );
    }
}

template <class T>
template <class... Attrs>
constexpr auto _AYU_DescribeBase<T>::attrs (const Attrs&... as) {
    return in::AttrsDcrWith<T, Attrs...>(as...);
}
template <class T>
template <class Acr>
constexpr auto _AYU_DescribeBase<T>::attr (
    OldStr key,
    const Acr& acr,
    in::AttrFlags flags
) {
     // Implicit member().
    if constexpr (std::is_member_object_pointer_v<Acr>) {
        return attr(key, _AYU_DescribeBase<T>::member(acr), flags);
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
template <class T>
template <class... Elems>
constexpr auto _AYU_DescribeBase<T>::elems (const Elems&... es) {
    return in::ElemsDcrWith<T, Elems...>(es...);
}
template <class T>
template <class Acr>
constexpr auto _AYU_DescribeBase<T>::elem (
    const Acr& acr,
    in::AttrFlags flags
) {
    if constexpr (std::is_member_object_pointer_v<Acr>) {
        return elem(_AYU_DescribeBase<T>::member(acr), flags);
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
template <class T>
template <class Acr>
constexpr auto _AYU_DescribeBase<T>::keys (const Acr& acr) {
    return in::KeysDcrWith<T, Acr>(acr);
}
template <class T>
constexpr auto _AYU_DescribeBase<T>::attr_func (Reference(* f )(T&, OldStr)) {
    return in::AttrFuncDcr<T>{{}, f};
}
template <class T>
template <class Acr>
constexpr auto _AYU_DescribeBase<T>::length (const Acr& acr) {
    return in::LengthDcrWith<T, Acr>(acr);
}
template <class T>
constexpr auto _AYU_DescribeBase<T>::elem_func (Reference(* f )(T&, usize)) {
    return in::ElemFuncDcr<T>{{}, f};
}
template <class T>
template <class Acr>
constexpr auto _AYU_DescribeBase<T>::delegate (const Acr& acr) {
    return in::DelegateDcrWith<T, Acr>(acr);
}

template <class T>
template <class T2, class M>
constexpr auto _AYU_DescribeBase<T>::member (
    M T2::* mp,
    in::AccessorFlags flags
) {
    return in::MemberAcr2<T, M>(mp, flags);
}
template <class T>
template <class T2, class M>
constexpr auto _AYU_DescribeBase<T>::const_member (
    const M T2::* mp,
    in::AccessorFlags flags
) {
    return in::MemberAcr2<T, M>(const_cast<M T::*>(mp), flags | in::ACR_READONLY);
}
template <class T>
template <class B>
    requires (requires (T* t, B* b) { b = t; t = static_cast<T*>(b); })
constexpr auto _AYU_DescribeBase<T>::base (
    in::AccessorFlags flags
) {
    return in::BaseAcr2<T, B>(flags);
}
template <class T>
template <class M>
constexpr auto _AYU_DescribeBase<T>::ref_func (
    M&(* f )(T&),
    in::AccessorFlags flags
) {
    return in::RefFuncAcr2<T, M>(f, flags);
}
template <class T>
template <class M>
constexpr auto _AYU_DescribeBase<T>::const_ref_func (
    const M&(* f )(const T&),
    in::AccessorFlags flags
) {
    return in::ConstRefFuncAcr2<T, M>(f, flags);
}
template <class T>
template <class M>
constexpr auto _AYU_DescribeBase<T>::const_ref_funcs (
    const M&(* g )(const T&),
    void(* s )(T&, const M&),
    in::AccessorFlags flags
) {
    return in::RefFuncsAcr2<T, M>(g, s, flags);
}
template <class T>
template <class M>
    requires (requires (M m) { M(std::move(m)); })
constexpr auto _AYU_DescribeBase<T>::value_func (
    M(* f )(const T&),
    in::AccessorFlags flags
) {
    return in::ValueFuncAcr2<T, M>(f, flags);
}
template <class T>
template <class M>
    requires (requires (M m) { M(std::move(m)); })
constexpr auto _AYU_DescribeBase<T>::value_funcs (
    M(* g )(const T&),
    void(* s )(T&, M),
    in::AccessorFlags flags
) {
    return in::ValueFuncsAcr2<T, M>(g, s, flags);
}
template <class T>
template <class M>
    requires (requires (M m) { M(std::move(m)); })
constexpr auto _AYU_DescribeBase<T>::mixed_funcs (
    M(* g )(const T&),
    void(* s )(T&, const M&),
    in::AccessorFlags flags
) {
    return in::MixedFuncsAcr2<T, M>(g, s, flags);
}

template <class T>
template <class M>
    requires (requires (T t, M m) { t = m; m = t; })
constexpr auto _AYU_DescribeBase<T>::assignable (
    in::AccessorFlags flags
) {
    return in::AssignableAcr2<T, M>(flags);
}

template <class T>
template <class M>
    requires (requires (M m) { M(std::move(m)); })
constexpr auto _AYU_DescribeBase<T>::constant (
    M&& v,
    in::AccessorFlags flags
) {
    return in::ConstantAcr2<T, M>(std::move(v), flags);
}
template <class T>
template <class M>
constexpr auto _AYU_DescribeBase<T>::constant_pointer (
    const M* p,
    in::AccessorFlags flags
) {
    return in::ConstantPointerAcr2<T, M>(p, flags);
}

 // This one is not constexpr, so it is only valid in attr_func, elem_func,
 // or reference_func.
template <class T>
template <class M>
    requires (requires (M m) { M(std::move(m)); m.~M(); })
auto _AYU_DescribeBase<T>::variable (
    M&& v,
    in::AccessorFlags flags
) {
    return in::VariableAcr2<T, M>(std::move(v), flags);
}

template <class T>
constexpr auto _AYU_DescribeBase<T>::reference_func (
    Reference(* f )(T&),
    in::AccessorFlags flags
) {
    return in::ReferenceFuncAcr2<T>(f, flags);
}

template <class T>
template <class... Dcrs>
constexpr auto _AYU_DescribeBase<T>::_ayu_describe (
    StaticString name, const Dcrs&... dcrs
) {
    return in::make_description<T, Dcrs...>(name, dcrs...);
}

} // namespace ayu

#ifdef AYU_DISCARD_ALL_DESCRIPTIONS
#define AYU_DESCRIBE(...)
#define AYU_DESCRIBE_0(...)
#define AYU_DESCRIBE_TEMPLATE(...)
#define AYU_DESCRIBE_INSTANTIATE(...)
#else

 // Stringify name as early as possible to avoid macro expansion
 // TODO make description constinit so names can be generated at runtime
#define AYU_DESCRIBE_BEGIN(T) AYU_DESCRIBE_BEGIN_NAME(T, #T)
#define AYU_DESCRIBE_BEGIN_NAME(T, name) \
template <> \
struct ayu_desc::_AYU_Describe<T> : ayu::_AYU_DescribeBase<T> { \
    using desc = ayu::_AYU_DescribeBase<T>; \
    static constexpr bool _ayu_defined = true; \
    static constexpr auto _ayu_full_description = ayu::_AYU_DescribeBase<T>::_ayu_describe(name,

#define AYU_DESCRIBE_END(T) \
    ); \
    static const ayu::in::Description* const _ayu_description; \
}; \
const ayu::in::Description* const ayu_desc::_AYU_Describe<T>::_ayu_description = \
    ayu::in::register_description( \
        _ayu_full_description.template get<ayu::in::Description>(0) \
    );

#define AYU_DESCRIBE(T, ...) AYU_DESCRIBE_NAME(T, #T, __VA_ARGS__)
#define AYU_DESCRIBE_NAME(T, name, ...) \
AYU_DESCRIBE_BEGIN_NAME(T, name) \
    __VA_ARGS__ \
AYU_DESCRIBE_END(T)

 // The only way to make an empty description work
 // TODO: use __VA_OPT__ instead
#define AYU_DESCRIBE_0(T) \
template <> \
struct ayu_desc::_AYU_Describe<T> : ayu::_AYU_DescribeBase<T> { \
    using desc = ayu::_AYU_DescribeBase<T>; \
    static constexpr bool _ayu_defined = true; \
    static constexpr auto _ayu_full_description = ayu::_AYU_DescribeBase<T>::_ayu_describe(#T); \
    static const ayu::in::Description* const _ayu_description; \
}; \
const ayu::in::Description* const ayu_desc::_AYU_Describe<T>::_ayu_description = \
    ayu::in::register_description( \
        _ayu_full_description.template get<ayu::in::Description>(0) \
    );

#define AYU_DESCRIBE_TEMPLATE_PARAMS(...) <__VA_ARGS__>
#define AYU_DESCRIBE_TEMPLATE_TYPE(...) __VA_ARGS__

#define AYU_DESCRIBE_TEMPLATE_BEGIN(params, T) \
template params \
struct ayu_desc::_AYU_Describe<T> : ayu::_AYU_DescribeBase<T> { \
    using desc = ayu::_AYU_DescribeBase<T>; \
    static constexpr bool _ayu_defined = true; \
    static constexpr auto _ayu_full_description = desc::_ayu_describe(ayu::OldStr(),

#define AYU_DESCRIBE_TEMPLATE_END(params, T) \
    ); \
    static const ayu::in::Description* const _ayu_description; \
}; \
template params \
const ayu::in::Description* const ayu_desc::_AYU_Describe<T>::_ayu_description = \
    ayu::in::register_description( \
        _ayu_full_description.template get<ayu::in::Description>(0) \
    );

#define AYU_DESCRIBE_ESCAPE(...) __VA_ARGS__

#define AYU_DESCRIBE_TEMPLATE(params, T, ...) \
AYU_DESCRIBE_TEMPLATE_BEGIN(AYU_DESCRIBE_ESCAPE(params), AYU_DESCRIBE_ESCAPE(T)) \
    __VA_ARGS__ \
AYU_DESCRIBE_TEMPLATE_END(AYU_DESCRIBE_ESCAPE(params), AYU_DESCRIBE_ESCAPE(T))

#define AYU_DESCRIBE_INSTANTIATE(T) \
static_assert(ayu_desc::_AYU_Describe<T>::_ayu_defined);

#define AYU_FRIEND_DESCRIBE(T) \
    friend struct ::ayu_desc::_AYU_Describe<T>;

#endif
