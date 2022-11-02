#pragma once

#include <chrono>

namespace uni {

inline double now () {
    using namespace std::chrono;
    return duration<double>(system_clock::now().time_since_epoch()).count();
}

} // namespace uni
