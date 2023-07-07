#pragma once

#include <type_traits>

namespace tap {

template <class F, class Ret, class... Args>
concept HasExactCallOperator = std::is_convertible_v<
    decltype(&F::operator()),
    Ret (F::*)(Args...)
>;

 // A super lightweight callback class with reference semantics (std::function
 // has value semantics and can be copied and moved, so it's way bigger.)
template <class> struct CallbackRefV;
template <class Ret, class... Args>
struct CallbackRefV<Ret(Args...)> {
    const void* f;
    Ret(* wrapper )(const void*, Args...);
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
     // GCC allows casting a method pointer to a function pointer, so take
     // advantage of that if we can.  This is not mainly for optimization (the
     // compiler does that pretty well already), it's to clean up the stack
     // for debugging.
    template <class F> requires(
        HasExactCallOperator<F, Ret, Args...>
    )
    [[gnu::always_inline]]
    constexpr CallbackRefV (const F& f) :
        f(&f),
        wrapper((decltype(wrapper))(&F::operator()))
    { }
    template <class F> requires(
        !HasExactCallOperator<F, Ret, Args...> &&
        std::is_convertible_v<std::invoke_result_t<F, Args...>, Ret>
    )
    [[gnu::always_inline]]
    constexpr CallbackRefV (const F& f) :
        f(&f),
        wrapper([](const void* f, Args... args)->Ret{
            return (*reinterpret_cast<const F*>(f))(std::forward<Args>(args)...);
        })
    { }
#pragma GCC diagnostic pop
#else
    template <class F> requires(
        std::is_convertible_v<std::invoke_result_t<F, Args...>, Ret>
    )
    [[gnu::always_inline]]
    constexpr CallbackRefV (const F& f) :
        f(&f),
        wrapper([](const void* f, Args... args)->Ret{
            return (*reinterpret_cast<const F*>(f))(std::forward<Args>(args)...);
        })
    { }
#endif
     // Looks like there's no way to avoid an extra copy of by-value args.
     // (std::function does it too)
    [[gnu::always_inline]]
    Ret operator () (Args... args) const {
        return wrapper(f, std::forward<Args>(args)...);
    }

    template <class Sig>
    const CallbackRefV<Sig>& reinterpret () const {
        return *(const CallbackRefV<Sig>*)this;
    }
};
 // Since CallbackRefV is two pointers long and trivially copyable, pass by value
 // instead of reference.
template <class Sig>
using CallbackRef = const CallbackRefV<Sig>;

} // namespace tap
