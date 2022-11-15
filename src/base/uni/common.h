#pragma once

#include <cassert>
#include <cstdint>
#include <cwchar>
#include <string>
#include <string_view>
#include <type_traits>

#include "../ayu/common.h"

namespace uni {
using namespace std::literals;

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
constexpr Null null = nullptr;

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

///// ASSERTIONS (use macros.h)

namespace X {
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
        String sdl_error;
        AssertionFailedSDL(Str function, Str filename, uint line, Str mess) :
            AssertionFailed(function, filename, line),
            sdl_error(mess)
        { }
    };
}

void assert_failed_general (const char* function, const char* filename, uint line);
void assert_failed_sdl (const char* function, const char* filename, uint line);

template <class T>
static constexpr T assert_general (
    T v, const char* function, const char* filename, uint line
) {
    if (!v) {
        if (std::is_constant_evaluated()) {
            throw X::AssertionFailed(function, filename, line);
        }
        else assert_failed_general(function, filename, line);
    }
    return v;
}
template <class T>
static constexpr T assert_sdl (
    T v, const char* function, const char* filename, uint line
) {
    if (!v) assert_failed_sdl(function, filename, line);
    return v;
}

} // namespace uni
