#pragma once

#include <cstdint>
#include <exception>
#include <limits>
#include <string>
#include <string_view>
#include <source_location>
#include <type_traits>
#include <utility>

namespace uni {
using namespace std::literals;

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using isize = std::intptr_t;
static_assert(std::is_same_v<isize, int32> || std::is_same_v<isize, int64>);
using uint = unsigned int;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using usize = std::uintptr_t;
static_assert(std::is_same_v<usize, uint32> || std::is_same_v<usize, uint64>);
 // I once met a compiler that defined uint32 as unsigned int, uint64 as
 // unsigned long long, but size_t as unsigned long.  If that happens, fiddle
 // with the compiler flags until it doesn't happen any more.
static_assert(std::is_same_v<usize, std::size_t>);

using Null = std::nullptr_t;
constexpr Null null = nullptr;

constexpr float nan = std::numeric_limits<float>::quiet_NaN();
constexpr float inf = std::numeric_limits<float>::infinity();

using String = std::string;
using Str = std::string_view;
using Str8 = std::u8string_view;
using Str16 = std::u16string_view;
using Str32 = std::u32string_view;
using WStr = std::wstring_view;

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

[[gnu::always_inline]]
static inline void undefined_behavior (
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
[[gnu::always_inline]]
static constexpr T&& require (T&& v, std::source_location loc) {
    if (!v) [[unlikely]] abort_requirement_failed(loc);
    return std::forward<T>(v);
}

template <class T>
[[gnu::always_inline]]
static constexpr T&& require_throw (T&& v, std::source_location loc) {
    if (!v) [[unlikely]] throw_requirement_failed(loc);
    return std::forward<T>(v);
}

template <class T>
[[gnu::always_inline]]
static constexpr T&& expect (T&& v, std::source_location loc) {
    if (!v) [[unlikely]] undefined_behavior(loc);
    return std::forward<T>(v);
}

} // namespace uni
