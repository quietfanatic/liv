#pragma once

#include <csignal>
#include <cstdint>
#include <cwchar>
#include <string>
#include <string_view>
#include <type_traits>

#include "../ayu/common.h"
#include "../ayu/exception.h"

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

using String = std::string;
using Str = std::string_view;
using Str8 = std::u8string_view;
using Str16 = std::u16string_view;
using Str32 = std::u32string_view;
using WStr = std::wstring_view;

///// ASSERTIONS (use macros.h)

struct AssertionFailed : ayu::Error {
     // TODO: Replace with std::source_location
    Str function;
    Str filename;
    uint line;
};

void assert_failed_general (const char* function, const char* filename, uint line);

template <class T>
static constexpr T&& assert_general (
    T&& v, const char* function, const char* filename, uint line
) {
    if (!v) {
        if (std::is_constant_evaluated()) {
            throw ayu::X<AssertionFailed>(function, filename, line);
        }
        else assert_failed_general(function, filename, line);
    }
    return std::forward<T>(v);
}

static inline void unreachable () {
     // It would be nice if __builtin_unreachable() would trap in debug builds,
     // but it doesn't.  Instead GCC just gives up on compiling the rest of the
     // function and lets the CPU run off the end, corrupting the stack and
     // making debugging difficult.
#if __GNUC__
#ifdef NDEBUG
    __builtin_unreachable();
#endif
    __builtin_trap();
#elif _MSC_VER
#ifdef NDEBUG
    __assume(false);
#endif
    __debug_break
#else
    std::abort();
#endif
}

template <class T>
static constexpr T&& debug_assert (T&& v) {
    if (!v) unreachable();
    return std::forward<T>(v);
}

} // namespace uni
