// Implements an ultra-simple hashing algorithm for strings.
// Uses x33 hash aka djb2

#pragma once

#include "common.h"

namespace uni {

template <class T>
constexpr uint64 fast_hash (T* s) {
    uint64 h = 0;
    for (; *s != 0; s++) {
        h = (h << 5) + h + uint64(*s);
    }
    return h;
}

 // For std::string-like objects
template <class T>
constexpr uint64 fast_hash (T s) {
    uint64 h = 0;
    for (auto c : s) {
        h = (h << 5) + h + uint64(c);
    }
    return h;
}

} // namespace uni
