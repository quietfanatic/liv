#pragma once

#include "common.h"

namespace glow {

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
};

} // namespace glow
