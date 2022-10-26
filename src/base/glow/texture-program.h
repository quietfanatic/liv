#pragma once

 // A super-simple gpu program that just draws a texture to the screen.
 // Not that useful, mainly for testing.

#include "../geo/rect.h"
#include "objects.h"

namespace glow {
using namespace geo;

void draw_texture (
    const Texture& tex,
    const Rect& screen_rect,
    const Rect& tex_rect = {0, 0, 1, 1}
);

} // namespace glow
