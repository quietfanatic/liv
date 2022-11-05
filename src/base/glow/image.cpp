#include "image.h"

#include <cerrno>
#include "../ayu/compat.h"
#include "../ayu/describe-standard.h"
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
        glBindTexture(target, id);
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

struct ImagePixelsProxy : Image { };

} using namespace glow;

AYU_DESCRIBE(glow::RGBA8,
    elems(
        elem(&RGBA8::r),
        elem(&RGBA8::g),
        elem(&RGBA8::b),
        elem(&RGBA8::a)
    )
)

AYU_DESCRIBE(glow::ImagePixelsProxy,
     // TODO: Allow parsing hex string as an option?
    length(value_funcs<usize>(
        [](const ImagePixelsProxy& image){
            return usize(area(image.size));
        },
        [](ImagePixelsProxy& image, usize len){
            AA(area(image.size) == isize(len));
            delete[](image.pixels);
            const_cast<RGBA8*&>(image.pixels) = new RGBA8 [area(image.size)];
        }
    )),
    elem_func([](ImagePixelsProxy& image, usize i){
        return ayu::Reference(&image.pixels[i]);
    })
)

AYU_DESCRIBE(glow::Image,
    attrs(
         // TODO: allocate here instead of in the proxy?
        attr("size", &Image::size),
        attr("pixels", ref_func<ImagePixelsProxy>(
            [](Image& img) -> ImagePixelsProxy& {
                return static_cast<ImagePixelsProxy&>(img);
            }
        ))
    )
)

AYU_DESCRIBE(glow::SubImage,
    attrs(
        attr("image", &SubImage::image),
        attr("bounds", &SubImage::bounds, optional)
    ),
    init([](SubImage& v){ v.validate(); })
)

AYU_DESCRIBE(glow::ImageTexture,
    attrs(
         // TODO: make inherit work in ayu
         // TODO: figure out how to make this optional without regenning texture
        attr("Texture", base<Texture>(), inherit),
        attr("SubImage", &ImageTexture::source, inherit),
        attr("flip", &ImageTexture::flip, optional),
        attr("internalformat", &ImageTexture::internalformat, optional)
    ),
    init([](ImageTexture& v){ v.init(); })
)

AYU_DESCRIBE(glow::X::ImageLoadFailed,
    delegate(base<X::GlowError>()),
    elems(
        elem(&X::ImageLoadFailed::filename),
        elem(&X::ImageLoadFailed::details)
    )
)

AYU_DESCRIBE(glow::X::ImageSaveFailed,
    delegate(base<X::GlowError>()),
    elems(
        elem(&X::ImageSaveFailed::filename),
        elem(&X::ImageSaveFailed::details)
    )
)
AYU_DESCRIBE(glow::X::SubImageBoundsNotProper,
    delegate(base<X::GlowError>()),
    elems( elem(&X::SubImageBoundsNotProper::bounds) )
)
AYU_DESCRIBE(glow::X::SubImageOutOfBounds,
    delegate(base<X::GlowError>()),
    elems(
        elem(&X::SubImageOutOfBounds::image),
        elem(&X::SubImageOutOfBounds::size),
        elem(&X::SubImageOutOfBounds::bounds)
    )
)
AYU_DESCRIBE(glow::X::ImageTextureIncompatibleTarget,
    delegate(base<X::GlowError>()),
    elems( elem(&X::ImageTextureIncompatibleTarget::target) )
)

