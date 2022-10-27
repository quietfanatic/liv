// An image type loaded through SDL_image and sent directly to and OpenGL
// texture.  The image pixels do not stay in CPU memory.

#pragma once

#include "base/geo/vec.h"
#include "base/glow/objects.h"
#include "base/uni/common.h"

struct ImageTexture : glow::Texture {
    geo::IVec size;
    ImageTexture (std::string filename);
    ~ImageTexture ();
};
