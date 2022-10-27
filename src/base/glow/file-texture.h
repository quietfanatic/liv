// A super basic image type loaded through SDL_image and sent directly to an
// OpenGL texture.  The image pixels do not stay in CPU memory.  Not haccable.

#pragma once

#include "../geo/vec.h"
#include "../uni/common.h"
#include "gl.h"
#include "objects.h"

namespace glow {

struct FileTexture : Texture {
     // Technically redundant since we can get the texture size through GL
     // but that's annoying and probably slow.
    geo::IVec size;
    FileTexture (std::string filename, uint32 target = GL_TEXTURE_RECTANGLE);
    ~FileTexture ();
};

} // namespace glow
