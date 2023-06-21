#pragma once

#include <charconv>
#include "arrays.h"

namespace uni {
inline namespace strings {

 // The string types are almost exactly the same as the equivalent array types.
 // The only differences are that they can be constructed from a const T*, which
 // is taken to be a C-style NUL-terminated string, and when constructing from a
 // C array they will stop at a NUL terminator (the first element that boolifies
 // to false).  Note that by default these strings are not NUL-terminated.  To
 // get a NUL-terminated string out, either explicitly NUL-terminate them or use
 // c_str().
template <class T>
using AnyGenericString = ArrayInterface<ArrayClass::AnyS, T>;
template <class T>
using SharedGenericString = ArrayInterface<ArrayClass::SharedS, T>;
template <class T>
using UniqueGenericString = ArrayInterface<ArrayClass::UniqueS, T>;
template <class T>
using StaticGenericString = ArrayInterface<ArrayClass::StaticS, T>;
template <class T>
using GenericStr = ArrayInterface<ArrayClass::SliceS, T>;

using AnyString = AnyGenericString<char>;
using SharedString = SharedGenericString<char>;
using UniqueString = UniqueGenericString<char>;
using StaticString = StaticGenericString<char>;
using Str = GenericStr<char>;

 // Literal suffix for StaticString.  This is probably not necessary, since
 // I finally figured out how to optimize passing string literals to cat().
consteval StaticString operator""_s (const char* p, usize s) {
    return StaticString::Static(p, s);
}

template <class T>
struct StringConversion;

template <>
struct StringConversion<char> {
    static constexpr usize min_capacity (char) { return 1; }
    static usize write (char* p, char v) {
        *p = v;
        return 1;
    }
};
template <>
struct StringConversion<bool> {
    static constexpr usize min_capacity (bool) { return 1; }
    static usize write (char* p, char v) {
        *p = v ? '1' : '0';
        return 1;
    }
};
template <>
struct StringConversion<uint64> {
    static constexpr usize min_capacity (uint64 v) {
        return v <= 999999 ? 6 : 19;
    }
    static usize write (char* p, uint64 v) {
        auto [ptr, ec] = std::to_chars(p, p + 19, v);
        expect(ec == std::errc());
        return ptr - p;
    }
};
template <>
struct StringConversion<int64> {
    static constexpr usize min_capacity (int64 v) {
        return uint64(v) <= 999999 ? 6 : 20;
    }
    static usize write (char* p, int64 v) {
        auto [ptr, ec] = std::to_chars(p, p + 20, v);
        expect(ec == std::errc());
        return ptr - p;
    }
};
template <>
struct StringConversion<uint8> {
    static constexpr usize min_capacity (uint8) { return 3; }
    ALWAYS_INLINE static usize write (char* p, uint8 v) {
        return StringConversion<uint64>::write(p, v);
    }
};
template <>
struct StringConversion<int8> {
    static constexpr usize min_capacity (int8) { return 4; }
    ALWAYS_INLINE static usize write (char* p, int8 v) {
        return StringConversion<int64>::write(p, v);
    }
};
template <>
struct StringConversion<uint16> {
    static constexpr usize min_capacity (uint16) { return 5; }
    ALWAYS_INLINE static usize write (char* p, uint16 v) {
        return StringConversion<uint64>::write(p, v);
    }
};
template <>
struct StringConversion<int16> {
    static constexpr usize min_capacity (int16) { return 6; }
    ALWAYS_INLINE static usize write (char* p, int16 v) {
        return StringConversion<int64>::write(p, v);
    }
};
template <>
struct StringConversion<uint32> {
    static constexpr usize min_capacity (uint32 v) {
        return v <= 9999 ? 4 : 10;
    }
    ALWAYS_INLINE static usize write (char* p, uint32 v) {
        return StringConversion<uint64>::write(p, v);
    }
};
template <>
struct StringConversion<int32> {
    static constexpr usize min_capacity (int32 v) {
        return uint32(v) <= 9999 ? 4 : 11;
    }
    ALWAYS_INLINE static usize write (char* p, int32 v) {
        return StringConversion<int64>::write(p, v);
    }
};
template <>
struct StringConversion<float> {
    static constexpr usize min_capacity (float) { return 16; }
    static usize write (char* p, float v) {
        if (v != v) {
            p[0] = '+'; p[1] = 'n'; p[2] = 'a'; p[3] = 'n';
            return 4;
        }
        else if (v == std::numeric_limits<float>::infinity()) {
            p[0] = '+'; p[1] = 'i'; p[2] = 'n'; p[3] = 'f';
            return 4;
        }
        else if (v == -std::numeric_limits<float>::infinity()) {
            p[0] = '-'; p[1] = 'i'; p[2] = 'n'; p[3] = 'f';
            return 4;
        }
        else [[likely]] {
            auto [ptr, ec] = std::to_chars(p, p + 16, v);
            expect(ec == std::errc());
            return ptr - p;
        }
    }
};
template <>
struct StringConversion<double> {
    static constexpr usize min_capacity (double) { return 24; }
    static usize write (char* p, double v) {
        if (v != v) {
            p[0] = '+'; p[1] = 'n'; p[2] = 'a'; p[3] = 'n';
            return 4;
        }
        else if (v == std::numeric_limits<double>::infinity()) {
            p[0] = '+'; p[1] = 'i'; p[2] = 'n'; p[3] = 'f';
            return 4;
        }
        else if (v == -std::numeric_limits<double>::infinity()) {
            p[0] = '-'; p[1] = 'i'; p[2] = 'n'; p[3] = 'f';
            return 4;
        }
        else [[likely]] {
            auto [ptr, ec] = std::to_chars(p, p + 24, v);
            expect(ec == std::errc());
            return ptr - p;
        }
    }
};
template <class T> requires (requires (T v) { GenericStr<char>(v); })
struct StringConversion<T> {
    static constexpr bool convert_to_Str = true;
    static constexpr usize min_capacity (const GenericStr<char>& v) {
        return v.size();
    }
    static usize write (char* p, const GenericStr<char>& v) {
        for (usize i = 0; i < v.size(); i++) {
            *p++ = v[i];
        }
        return v.size();
    }
};
 // Conversion for C char arrays, which are most likely string literals.
template <usize n>
struct StringConversion<char[n]> {
    using char_n = char[n];
    static constexpr usize min_capacity (const char_n& v) {
         // Avoid loop and possibly overestimate.
        return n > 0 && v[n-1] == 0 ? n-1 : n;
    }
    static usize write (char* p, const char_n& v) {
         // Stop writing at a NUL terminator.
        usize i;
        for (i = 0; i < n && v[i]; i++) {
            *p++ = v[i];
        }
        return i;
    }
};

namespace in {

 // If we don't add this expect(), the compiler emits extra branches for when
 // the total size overflows to 0, but those branches just crash anyway.
constexpr
void cat_add_no_overflow (usize& a, usize b) {
    expect(a + b <= UniqueString::max_size_);
    a += b;
}

template <class... Tail> inline
void cat_append (
    ArrayImplementation<ArrayClass::UniqueS, char>& h, Tail&&... t
) {
    if constexpr (sizeof...(Tail) > 0) {
        UniqueString& s = reinterpret_cast<UniqueString&>(h);
         // reserve
        usize total_size = h.size;
        (cat_add_no_overflow(total_size, StringConversion<
            std::remove_cvref_t<Tail>
        >::min_capacity(t)), ...);
        s.reserve_plenty(total_size);
         // write
        ((h.size += StringConversion<
            std::remove_cvref_t<Tail>
        >::write(h.data + h.size, t)), ...);
    }
}

 // Extra early conversion step to avoid having to call strlen() twice
template <class T> ALWAYS_INLINE
GenericStr<char> cat_convert (const T& v) requires (
    requires { StringConversion<T>::convert_to_Str; }
) {
    return GenericStr<char>(v);
}
template <class T> ALWAYS_INLINE
const T& cat_convert (const T& v) { return v; }

} // in

 // Concatenation for character strings.  Returns the result of printing all the
 // arguments, concatenated into a single string.
template <class Head, class... Tail> inline
UniqueString cat (Head&& h, Tail&&... t) {
    if constexpr (
        std::is_same_v<Head&&, UniqueString&&> ||
        std::is_same_v<Head&&, SharedString&&> ||
        std::is_same_v<Head&&, AnyString&&>
    ) {
        if (h.unique()) {
            ArrayImplementation<ArrayClass::UniqueS, char> impl;
            impl.size = h.size(); impl.data = h.data();
            h.dematerialize();
            in::cat_append(impl, in::cat_convert(t)...);
            return UniqueString::Materialize(impl.data, impl.size);
        }
    }
    ArrayImplementation<ArrayClass::UniqueS, char> impl = {};
    in::cat_append(impl, in::cat_convert(h), in::cat_convert(t)...);
    return UniqueString::Materialize(impl.data, impl.size);
}

} // strings
} // uni

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"
namespace tap {
template <uni::ArrayClass ac>
struct Show<uni::ArrayInterface<ac, char>> {
    std::string show (const uni::ArrayInterface<ac, char>& v) {
        return std::string(uni::cat("\"", v, "\""));
    }
};
}
#endif
