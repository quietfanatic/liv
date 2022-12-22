#pragma once

#include "../ayu/common.h"
#include "../uni/common.h"
#include "../uni/macros.h"

namespace glow {
using namespace uni;

void init ();

struct GlowError : ayu::Error { };

struct AssertionFailedSDL : AssertionFailed {
    String sdl_error;
};

void assert_failed_sdl (const char* function, const char* filename, uint line);

template <class T>
static constexpr T&& assert_sdl (
    T&& v, const char* function, const char* filename, uint line
) {
    if (!v) assert_failed_sdl(function, filename, line);
    return std::forward<T>(v);
}
#define AS(v) ::glow::assert_sdl(v, __FUNCTION__, __FILE__, __LINE__)

} // namespace glow
