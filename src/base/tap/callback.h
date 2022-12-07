#pragma once

#include <type_traits>

namespace tap {

#if __GNUC__ && __cpp_concepts
template <class F, class Ret, class... Args>
concept IsMFP = requires { &F::operator(); } &&
    std::is_same_v<std::invoke_result_t<F, Args...>, Ret>;
#endif

 // A super lightweight callback class with reference semantics (std::function
 // has value semantics and can be copied and moved, so it's way bigger.)
template <class> struct CallbackV;
template <class Ret, class... Args>
struct CallbackV<Ret(Args...)> {
    Ret(* wrapper )(const void*, Args...);
    const void* f;
#if __GNUC__ && __cpp_concepts
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
     // GCC allows casting a method pointer to a function pointer, so take
     // advantage of that if we can.  This is not mainly for optimization (the
     // compiler does that pretty well already), it's to clean up the stack
     // for debugging.
     // TODO: Reject raw function pointer here
    template <class F> requires(
        IsMFP<F, Ret, Args...>
    )
    [[gnu::always_inline]]
    constexpr CallbackV (const F& f) :
        wrapper((decltype(wrapper))(&F::operator())),
        f(&f)
    { }
    template <class F> requires(
        !IsMFP<F, Ret, Args...> &&
        std::is_convertible_v<std::invoke_result_t<F, Args...>, Ret>
    )
    [[gnu::always_inline]]
    constexpr CallbackV (const F& f) :
        wrapper([](const void* f, Args... args)->Ret{
            return (*reinterpret_cast<const F*>(f))(std::forward<Args>(args)...);
        }),
        f(&f)
    { }
#pragma GCC diagnostic pop
#else
    template <class F>
    [[gnu::always_inline]]
    constexpr CallbackV (const F& f) :
        wrapper([](const void* f, Args... args)->Ret{
            return (*reinterpret_cast<const F*>(f))(std::forward<Args>(args)...);
        }),
        f(&f)
    { }
#endif
     // Looks like there's no way to avoid an extra copy of by-value args.
     // (std::function does it too)
    [[gnu::always_inline]]
    Ret operator () (Args... args) const {
        return wrapper(f, std::forward<Args>(args)...);
    }
};
template <class Sig>
using Callback = const CallbackV<Sig>&;

} // namespace tap
