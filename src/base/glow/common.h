#pragma once

#include "../ayu/common.h"
#include "../uni/common.h"
#include "../uni/macros.h"

namespace glow {
using namespace uni;

void init ();

struct GlowError : ayu::Error { };

[[noreturn]]
void requirement_failed_sdl (std::source_location loc = std::source_location::current());

template <class T>
[[gnu::always_inline]]
static constexpr T&& require_sdl (
    T&& v, std::source_location loc = std::source_location::current()
) {
    if (!v) [[unlikely]] requirement_failed_sdl(loc);
    return std::forward<T>(v);
}

} // namespace glow
