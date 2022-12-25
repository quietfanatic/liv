#pragma once
#include <exception>
#include <string>
#include <source_location>
#include "common.h"

namespace uni {
inline namespace requirements {

 // Abort if the condition isn't true.
template <class T>
static constexpr T&& require (
    T&& v, std::source_location loc = std::source_location::current()
);

 // Throw if the condition isn't true
template <class T>
static constexpr T&& require_throw (
    T&& v, std::source_location loc = std::source_location::current()
);

 // Either aborts or triggers undefined behavior if the condition isn't true,
 // depending on NDEBUG.  Always evaluates the argument in either case.  If the
 // argument can't be optimized out, check NDEBUG yourself.
template <class T>
static constexpr T&& expect (
    T&& v, std::source_location loc = std::source_location::current()
);

 // The exception thrown by require_throw
struct RequirementFailed : std::exception {
    std::source_location loc;
    RequirementFailed (
        std::source_location loc = std::source_location::current()
    ) : loc(loc) { }
    mutable std::string mess_cache;
    const char* what () const noexcept override;
};

[[noreturn]]
void throw_requirement_failed (std::source_location = std::source_location::current());
[[noreturn]]
void abort_requirement_failed (std::source_location = std::source_location::current());

 // Equivalent to expect(false) but doesn't warn about lack of return
[[noreturn]]
static inline void never (
    [[maybe_unused]] std::source_location loc = std::source_location::current()
) {
#ifdef NDEBUG
#if __GNUC__
    __builtin_unreachable();
#elif _MSC_VER
    __assume(false);
#else
    *(int*)null = 0;
#endif
#else
    abort_requirement_failed(loc);
#endif
}

template <class T>
ALWAYS_INLINE static constexpr T&& require (T&& v, std::source_location loc) {
    if (!v) [[unlikely]] abort_requirement_failed(loc);
    return std::forward<T>(v);
}

template <class T>
ALWAYS_INLINE static constexpr T&& require_throw (T&& v, std::source_location loc) {
    if (!v) [[unlikely]] throw_requirement_failed(loc);
    return std::forward<T>(v);
}

template <class T>
ALWAYS_INLINE static constexpr T&& expect (T&& v, std::source_location loc) {
    if (!v) [[unlikely]] never(loc);
    return std::forward<T>(v);
}

} // requirements
} // uni
