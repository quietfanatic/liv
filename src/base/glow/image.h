#pragma once

#include <optional>
#include "../geo/rect.h"
#include "../geo/vec.h"
#include "colors.h"
#include "objects.h"

namespace glow {
using namespace geo;

 // This type is not directly serializable.  Instead, you can get it from
 //  Image* something = hacc::Resource("something.png").ref();
 // The pixels are stored so that these coordinates are in order:
 //  {0, 0}, {1, 0}, {0, 1}, {1, 1}
 // In other words, the way you expect.  However, image file formats and OpenGL
 //  disagree on whether y goes up or down, so if you're martialling this image
 //  into a texture, it will likely have to be flipped at some point.
struct Image {
    const IVec size;
    RGBA8* const pixels;

    CE Image () : pixels(null) { }
     // Create from already-allocated pixels.
    Image (IVec s, RGBA8*&& p) : size(s), pixels(p) { p = nullptr; }
     // Allocate new pixels array
    explicit Image (IVec size);
     // Load from PNG file.  The top-left corner of the image will be at {0,0}.
    explicit Image (Str filename);

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
     // A rectangle containing all valid indexes
    IRect index_bounds () const {
        return {{0, 0}, size - 1};
    }
    RGBA8& operator [] (IVec i) {
        AA(pixels);
        AA(contains(index_bounds(), i));
        return pixels[i.y * size.x + i.x];
    }
    const RGBA8& operator [] (IVec i) const {
        AA(pixels);
        AA(contains(index_bounds(), i));
        return pixels[i.y * size.x + i.x];
    }

    void save (Str filename) const;
};

 // Const reference type that refers to a portion of another image.
struct SubImage {
     // Image that is being referenced.
    const Image* image = null;
     // Area of the image in pixels.  Coordinates refer to the corners between
     //  pixels, not the pixels themselves.  If null, refers to the entire
     //  image.  Otherwise cannot have negative width or height and cannot be
     //  ourside the bounds of the image.
    std::optional<IRect> bounds = std::nullopt;

     // Will throw if bounds is outside the image or is not proper.
     // Can't check if the bounds or image size is changed later.
    void validate ();

    CE SubImage () { }
    explicit SubImage (const Image* image, const std::optional<IRect>& bounds = std::nullopt) :
        image(image), bounds(bounds)
    { validate(); }

    CE explicit operator bool () { return image && *image; }

    IVec size () const {
        if (bounds) return bounds->size();
        else {
            AA(image);
            return image->size;
        }
    }
    IRect index_bounds () const {
        return {{0, 0}, size() - 1};
    }
    const RGBA8& operator [] (IVec i) const {
        AA(image);
        AA(contains(index_bounds(), i));
        return (*image)[bounds ? i + bounds->lb() : i];
    }
};

 // Represents a texture loaded from an image.
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
