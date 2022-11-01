#pragma once

#include "common.h"
#include "../geo/vec.h"

namespace glow {

 // TODO: Rename this to texture.h

 // A texture in video memory.
 // glGenTextures will be called on construction and glDeleteTextures on
 //  destruction.
struct Texture {
     // Specifies what kind of texture this is.  GL_TEXTURE_*.
     // If 0, texture won't actually be created.
    const uint target;

    explicit Texture (uint target = 0);

    Texture (Texture&& o) : target(o.target), id(o.id) {
        const_cast<uint&>(o.id) = 0;
    }
    ~Texture ();
    ASSIGN_BY_MOVE(Texture)

    const uint id = 0;
    operator uint () const { return id; }

     // Uses glGetTexLevelParameter
     // Returns {0, 0} if this texture (level) has not been initialized
    geo::IVec size (int level = 0);
     // Returns 0 if this texture (level) has not been initialized
     // I believe this can return a maximum of 256 (double precision RGBA)
    int32 bpp (int level = 0);
};

} // namespace glow
