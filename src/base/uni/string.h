// Contains utilities for string handling.  Will eventually contain an actual
// string type
#pragma once

namespace uni {

using String = std::string;
using String8 = std::u8string;
using String16 = std::u16string;
using String32 = std::u32string;
using WString = std::wstring;

namespace in {
    template <class T>
    static auto to_string (T&& s) {
        if constexpr (
            !std::is_same_v<std::decay_t<T>, char>
            && requires (T v) { std::to_string(v); }
        ) return std::to_string(std::forward<T>(s));
        else return std::forward<T>(s);
    }
}

 // I'm tired of all the weirdness around string concatenation operators.
 // Just use this instead.
template <class... Args>
String cat (Args&&... args) {
    String r; // Should we reserve()?  Profile!
    ((r += in::to_string(std::forward<Args>(args))), ...);
    return r;
}
 // Optimization to skip a copy
template <class... Args>
String cat (String&& s, Args... args) {
    String r = std::move(s);
    ((r += in::to_string(std::forward<Args>(args))), ...);
    return r;
}
}
