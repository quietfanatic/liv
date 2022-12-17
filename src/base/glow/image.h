#pragma once

#include <optional>
#include "../geo/rect.h"
#include "../geo/vec.h"
#include "colors.h"
#include "objects.h"

namespace glow {
using namespace geo;

 // This type is not directly serializable.  Instead, you can get it from
 //  Image* something = ayu::Resource("something.png").ref();
 // The pixels are stored so that these coordinates are in order:
 //  {0, 0}, {1, 0}, {0, 1}, {1, 1}
 // In other words, the way you expect.  However, image file formats and OpenGL
 //  disagree on whether y goes up or down, so if you're martialling this image
 //  into a texture, it will likely have to be flipped at some point.
struct Image {
    const IVec size;
     // The allocation method of this is not specified, so if you take this
     // pointer somewhere else, be sure to return it to an Image to deallocate
     // it.
    RGBA8* const pixels;

    CE Image () : pixels(null) { }
     // Create from already-allocated pixels.
    Image (IVec s, RGBA8*&& p) : size(s), pixels(p) { p = nullptr; }
     // Allocate new pixels array
    explicit Image (IVec size);

    Image (Image&& o) : size(o.size), pixels(o.pixels) {
        const_cast<RGBA8*&>(o.pixels) = null;
    }
    ~Image ();
    ASSIGN_BY_MOVE(Image)

    CE explicit operator bool () const { return pixels; }

     // A rectangle containins all square pixels
    IRect bounds () const {
        return {{0, 0}, size};
    }
    RGBA8& operator [] (IVec i) {
        DA(pixels);
        DA(contains(bounds(), i));
        return pixels[i.y * size.x + i.x];
    }
    const RGBA8& operator [] (IVec i) const {
        DA(pixels);
        DA(contains(bounds(), i));
        return pixels[i.y * size.x + i.x];
    }
};

 // Const reference type that refers to a portion of another image.
struct SubImage {
     // Image that is being referenced.
    const Image* image = null;
     // Area of the image in pixels.  Coordinates refer to the corners between
     // pixels, not the pixels themselves.  As a special case, GINF refers to
     // the entire image.  Otherwise, cannot have negative width or height and
     // cannot be outside the bounds of the image.
    IRect bounds = GINF;

     // Will throw if bounds is outside the image or is not proper.
     // Can't check if the bounds or image size is changed later.
    void validate ();

    CE SubImage () { }
    explicit SubImage (const Image* image, const IRect& bounds = GINF) :
        image(image), bounds(bounds)
    { validate(); }

    CE explicit operator bool () { return image && *image; }

    IVec size () const {
        if (bounds != GINF) return geo::size(bounds);
        else {
            DA(image);
            return image->size;
        }
    }
    const RGBA8& operator [] (IVec i) const {
        DA(image);
        DA(contains(bounds != GINF ? bounds : IRect({0, 0}, image->size), i));
        return (*image)[bounds != GINF ? i + lb(bounds) : i];
    }
};

 // Represents a texture loaded from an image.  Does not automatically support mipmaps.
struct ImageTexture : Texture {
    SubImage source;
     // TODO: move back to SubImage?
    BVec flip;
    uint internalformat;
     // Supported targets: GL_TEXTURE_2D, GL_TEXTURE_1D_ARRAY, GL_TEXTURE_RECTANGLE
    explicit ImageTexture (
        uint target = 0,
        const SubImage& subimage = {},
        BVec flip = {false, true},  // Flip vertically by default
        uint internalformat = 0x1908  // GL_RGBA
    ) :
        Texture(target), source(subimage), flip(flip), internalformat(internalformat)
    {
        init();
    }
     // (Re)uploads texture if target is not 0.
    void init ();
};

namespace X {
    struct ImageLoadFailed : GlowError {
        String filename;
        String details;
        ImageLoadFailed (Str f, String&& d) : filename(f), details(d) { }
    };
    struct ImageSaveFailed : GlowError {
        String filename;
        String details;
        ImageSaveFailed (Str f, String&& d) : filename(f), details(d) { }
    };
    struct SubImageBoundsNotProper : GlowError {
        IRect bounds;
        SubImageBoundsNotProper (const IRect& b) : bounds(b) { }
    };
    struct SubImageOutOfBounds : GlowError {
        const Image* image;
        IVec size;
        IRect bounds;
        SubImageOutOfBounds (const Image* i, IVec s, const IRect& b) : image(i), size(s), bounds(b) { }
    };
    struct ImageTextureIncompatibleTarget : GlowError {
        uint target;
        ImageTextureIncompatibleTarget (uint t) : target(t) { }
    };
}

}
