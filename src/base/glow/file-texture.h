// A super basic image type loaded through SDL_image and sent directly to an
// OpenGL texture.  The image pixels do not stay in CPU memory.  Not haccable.
// Does not support mipmaps (please set filtering to a non-mipmap mode).

#pragma once

#include "../geo/vec.h"
#include "../uni/common.h"
#include "gl.h"
#include "objects.h"

namespace glow {

struct FileTexture : Texture {
    FileTexture (std::string filename, uint32 target = GL_TEXTURE_2D);
    ~FileTexture ();
};

} // namespace glow
