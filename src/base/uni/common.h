#pragma once

#include <cassert>
#include <cstdint>
#include <cwchar>
#include <string>
#include <string_view>
#include <type_traits>

#include "../ayu/common.h"

///// WHATEVER
using namespace std::literals;

///// CONVENIENCE MACROS

#define CE constexpr

///// TYPES

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using isize = std::intptr_t;
using uint = unsigned int;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using usize = std::uintptr_t;

using Null = std::nullptr_t;
CE Null null = nullptr;

using String = std::string;
using Str = std::string_view;
 // This is scuffed, wide strings are all scuffed
#if WCHAR_MAX > 0xffff
using String16 = std::u16string;
using Str16 = std::u16string_view;
#else
using String16 = std::wstring;
using Str16 = std::wstring_view;
#endif

 // I guess these aren't in the standard library because they increase the risk
 // of referencing a temporary.  So just don't do that. :)
inline String operator + (Str a, Str b) {
    String r;
    r.reserve(a.size() + b.size());
    return (r += a) += b;
}
inline String operator + (Str a, char b) {
    String r;
    r.reserve(a.size() + 1);
    return (r += a) += b;
}
inline String operator + (char a, Str b) {
    String r;
    r.reserve(1 + b.size());
    return (r += a) += b;
}
inline String operator + (String&& a, Str b) {
     // Optimization
    return a += b;
}
inline String16 operator + (Str16 a, Str16 b) {
    String16 r;
    r.reserve(a.size() + b.size());
    return (r += a) += b;
}
inline String16 operator + (Str16 a, wchar_t b) {
    String16 r;
    r.reserve(a.size() + 1);
    return (r += a) += b;
}
inline String16 operator + (wchar_t a, Str16 b) {
    String16 r;
    r.reserve(1 + b.size());
    return (r += a) += b;
}
inline String16 operator + (String16&& a, Str16 b) {
     // Optimization
    return a += b;
}

///// CLASS DEFINITION CONVENIENCE
 // Recommended that you make sure the move and copy constructors can't throw.
#define ASSIGN_BY_MOVE(T) \
T& operator = (T&& o) { \
    this->~T(); \
    new (this) T (std::move(o)); \
    return *this; \
}
#define ASSIGN_BY_COPY(T) \
T& operator = (const T& o) { \
    this->~T(); \
    new (this) T (o); \
    return *this; \
}

///// ASSERTIONS

void assert_failed_general (const char* function, const char* filename, uint line);
void assert_failed_sdl (const char* function, const char* filename, uint line);

template <class T>
static CE T assert_general (T v, const char* function, const char* filename, uint line) {
    if (std::is_constant_evaluated()) {
        assert(v);
    }
    else {
        if (!v) assert_failed_general(function, filename, line);
    }
    return v;
}
template <class T>
static CE T assert_sdl (T v, const char* function, const char* filename, uint line) {
    if (!v) assert_failed_sdl(function, filename, line);
    return v;
}

#define AA(v) assert_general(v, __FUNCTION__, __FILE__, __LINE__)
#ifdef NDEBUG
#define DA(v)
#else
#define DA(v) AA(v)
#endif
#define AS(v) assert_sdl(v, __FUNCTION__, __FILE__, __LINE__)

namespace uni::X {
    struct AssertionFailed : ayu::X::Error {
        String function;
        String filename;
        uint line;
        AssertionFailed(Str function, Str filename, uint line) :
            function(function),
            filename(filename),
            line(line)
        { }
    };
    struct AssertionFailedSDL : AssertionFailed {
        String mess;
        AssertionFailedSDL(Str function, Str filename, uint line, Str mess) :
            AssertionFailed(function, filename, line),
            mess(mess)
        { }
    };
}
