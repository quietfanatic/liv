#include "image.h"

#include <cerrno>
#include "../hacc/compat.h"
#include "../hacc/haccable-standard.h"
#include "../hacc/resource.h"
#include "gl.h"

namespace glow {

Image::Image (IVec s) :
    size((AA(area(s) >= 0), s)), pixels(new RGBA8 [area(size)])
{ }

Image::~Image () { delete[](pixels); }

void SubImage::validate () {
    if (!bounds) return;
    if (!proper(*bounds)) throw X::SubImageBoundsNotProper(*bounds);
    if (image && *bounds != INF && !contains(image->bounds(), *bounds)) {
        throw X::SubImageOutOfBounds(image, image->size, *bounds);
    }
}

void ImageTexture::init () {
    if (target && source) {
        AA(target == GL_TEXTURE_2D
            || target == GL_TEXTURE_1D_ARRAY
            || target == GL_TEXTURE_RECTANGLE
        );
        Image processed (source.size());
        IRect ib = source.index_bounds();
        for (int y = 0; y < processed.size.y; y++)
        for (int x = 0; x < processed.size.x; x++) {
            processed[{x, y}] = source[{
                flip.x ? ib.r - x : ib.l + x,
                flip.y ? ib.t - y : ib.b + y
            }];
        }
        glTexImage2D(
            target,
            0, // level
            internalformat,
            processed.size.x,
            processed.size.y,
            0, // border
            GL_RGBA, // format
            GL_UNSIGNED_BYTE, // type
            processed.pixels
        );
    }
}

} using namespace glow;

HACCABLE(glow::RGBA8,
    elems(
        elem(&RGBA8::r),
        elem(&RGBA8::g),
        elem(&RGBA8::b),
        elem(&RGBA8::a)
    )
)

HACCABLE(glow::Image,
    attrs(
         // TODO: allocate
        attr("size", &Image::size),
         // TODO: allow more than one data input method?  Either through
         // a proxy type or by adding a "redundant" attr property to haccable
        attr("pixels", mixed_funcs<std::vector<RGBA8>>(
            [](const glow::Image& img){
                AA(area(img.size) >= 0);
                std::vector<RGBA8> pixels (area(img.size));
                for (isize i = 0; i < area(img.size); i++) {
                    pixels[i] = img.pixels[i];
                }
                return pixels;
            },
            [](glow::Image& img, const std::vector<RGBA8>& pixels){
                AA(isize(pixels.size()) == area(img.size));
                 // TODO: don't free
                if (img.pixels) delete[](img.pixels);
                const_cast<RGBA8*&>(img.pixels) = new RGBA8 [area(img.size)];
                for (isize i = 0; i < area(img.size); i++) {
                    img.pixels[i] = pixels[i];
                }
            }
        ))
    )
)

HACCABLE(glow::SubImage,
    attrs(
        attr("image", &SubImage::image),
        attr("bounds", &SubImage::bounds, optional)
    ),
    init([](SubImage& v){ v.validate(); })
)

HACCABLE(glow::ImageTexture,
    attrs(
         // TODO: make inherit work in hacc
         // TODO: figure out how to make this optional without regenning texture
        attr("Texture", base<Texture>(), inherit),
        attr("SubImage", &ImageTexture::source, inherit),
        attr("flip", &ImageTexture::flip, optional),
        attr("internalformat", &ImageTexture::internalformat, optional)
    ),
    init([](ImageTexture& v){ v.init(); })
)

HACCABLE(glow::X::ImageLoadFailed,
    elems(
        elem(&X::ImageLoadFailed::filename),
        elem(&X::ImageLoadFailed::details)
    )
)

HACCABLE(glow::X::ImageSaveFailed,
    elems(
        elem(&X::ImageSaveFailed::filename),
        elem(&X::ImageSaveFailed::details)
    )
)
HACCABLE(glow::X::SubImageBoundsNotProper,
    elems( elem(&X::SubImageBoundsNotProper::bounds) )
)
HACCABLE(glow::X::SubImageOutOfBounds,
    elems(
        elem(&X::SubImageOutOfBounds::image),
        elem(&X::SubImageOutOfBounds::size),
        elem(&X::SubImageOutOfBounds::bounds)
    )
)
HACCABLE(glow::X::ImageTextureIncompatibleTarget,
    elems( elem(&X::ImageTextureIncompatibleTarget::target) )
)

