#pragma once

#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

namespace uni {
inline namespace common {
using namespace std::literals;

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using isize = std::intptr_t;
using uint = unsigned int;  // 32 bits on most platforms
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using usize = std::uintptr_t;
 // I once met a compiler that defined uint32 as unsigned int, uint64 as
 // unsigned long long, but size_t as unsigned long.  If that happens, fiddle
 // with the compiler flags until it doesn't.
static_assert(std::is_same_v<isize, int32> || std::is_same_v<isize, int64>);
static_assert(std::is_same_v<usize, uint32> || std::is_same_v<usize, uint64>);
static_assert(std::is_same_v<usize, std::size_t>);

using Null = std::nullptr_t;
constexpr Null null = nullptr;

#ifdef nan
#warning "Somebody defined nan as a macro, undefining it"
#undef nan
#endif
constexpr float nan = std::numeric_limits<float>::quiet_NaN();
#ifdef inf
#warning "Somebody defined inf as a macro, undefining it"
#undef inf
#endif
constexpr float inf = std::numeric_limits<float>::infinity();

using OldStr = std::string_view;
using OldStr8 = std::u8string_view;
using OldStr16 = std::u16string_view;
using OldStr32 = std::u32string_view;
using OldWStr = std::wstring_view;

using std::move;

} // common
} // uni

///// MACROS

#ifndef HAS_BUILTIN
    #ifdef __has_builtin
        #define HAS_BUILTIN(f) __has_builtin(f)
    #else
        #define HAS_BUILTIN(f)
    #endif
#endif

 // These need to be before constexpr
#ifndef ALWAYS_INLINE
    #if __GNUC__
        #define ALWAYS_INLINE [[gnu::artificial]] inline
    #elif _MSC_VER
        #define ALWAYS_INLINE __forceinline
    #else
        #define ALWAYS_INLINE inline
    #endif
#endif

#ifndef NOINLINE
    #if __GNUC__
        #define NOINLINE [[gnu::noinline]]
    #elif _MSC_VER
        #define NOINLINE __declspec(noinline)
    #else
        #define NOINLINE
    #endif
#endif

 // Automatically define assignment operators based on constructors.
 // WARNING: If the constructor throws an exception, the lvalue will be left
 // uninitialized.
#define ASSIGN_BY_MOVE(T) \
    T& operator= (T&& o) { \
        this->~T(); \
        return *new (this) T(std::move(o)); \
    }
#define ASSIGN_BY_COPY(T) \
    T& operator= (const T& o) { \
        this->~T(); \
        return *new (this) T(o); \
    }
