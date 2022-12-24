// Contains utilities for string handling.  Will eventually contain an actual
// string type
#pragma once

#include <type_traits>
#include "common.h"

namespace uni {

namespace in {
    template <class T>
    static auto to_string (T&& s) {
        if CE (
            !std::is_same_v<std::decay_t<T>, char>
            && requires (T v) { std::to_string(v); }
        ) return std::to_string(std::forward<T>(s));
        else return std::forward<T>(s);
    }
}

 // I'm tired of all the weirdness around string concatenation operators.
 // Just use this instead.
template <class... Args>
std::string cat (Args&&... args) {
    std::string r; // Should we reserve()?  Profile!
    ((r += in::to_string(std::forward<Args>(args))), ...);
    return r;
}
 // Optimization to skip a copy
template <class... Args>
std::string cat (std::string&& s, Args... args) {
    std::string r = std::move(s);
    ((r += in::to_string(std::forward<Args>(args))), ...);
    return r;
}

} // namespace uni
