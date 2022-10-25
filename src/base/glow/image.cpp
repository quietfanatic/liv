#include "image.h"

#include <cerrno>
#include "../hacc/compat.h"
#include "../hacc/haccable-standard.h"
#include "../hacc/resource.h"
#include "../../external/lodepng/lodepng.h"
#include "gl.h"

using namespace std::literals;

namespace glow {

Image::Image (IVec s) : size((AA(area(s) >= 0), s)), pixels(
    reinterpret_cast<RGBA8*>(malloc(area(size) * sizeof(RGBA8)))
) { }
Image::Image (Str filename) : size(NAN), pixels(null) {
    unsigned char* data = null;
    unsigned w, h;
    AA(filename[filename.size()] == 0);
    auto err = lodepng_decode32_file(&data, &w, &h, filename.data());
    if (err) {
        free(data);
        throw X::ImageLoadFailed(filename, lodepng_error_text(err));
    }
    const_cast<IVec&>(size) = IVec{w, h};
    const_cast<RGBA8*&>(pixels) = reinterpret_cast<RGBA8*>(data);
}

Image::~Image () { free(pixels); }

void Image::save (Str filename) const {
    AA(pixels);
    AA(filename[filename.size()] == 0);
    auto err = lodepng_encode32_file(
        filename.data(),
        reinterpret_cast<uint8*>(pixels),
        size.x, size.y
    );
    if (err) {
        throw X::ImageSaveFailed(filename, lodepng_error_text(err));
    }
}

struct PNGResourceHandler : hacc::ResourceHandler {
    bool ResourceHandler_can_handle (hacc::Resource res) override {
        auto name = res.name();
        auto suffix = ".png"sv;
        return name.size() >= suffix.size()
            && name.substr(name.size() - suffix.size()) == suffix;
    }
    void ResourceHandler_load (hacc::Resource res) override {
        auto filename = hacc::resource_filename(res.name());
        res.set_value() = Image(filename);
    }
    std::function<void()> ResourceHandler_save (hacc::Resource res) override {
        auto filename = hacc::resource_filename(res.name());
        Image* img = res.ref();
        AA(img->pixels);
        return [img, filename{std::move(filename)}]{ img->save(filename); };
    }
    void ResourceHandler_remove_source (hacc::Resource res) override {
        auto filename = hacc::resource_filename(res.name());
        if (hacc::remove_utf8(filename.c_str()) != 0) {
            if (errno != ENOENT) {
                throw hacc::X::RemoveSourceFailed(res, errno);
            }
        }
    }
};

static PNGResourceHandler png_resource_handler;

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

 // TODO
HACCABLE_0(glow::Image)

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

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"

static tap::TestSet tests ("base/glow/image", []{
    using namespace hacc;
    using namespace tap;
    Image* test_image = nullptr;
    auto image_res = Resource("/base/glow/test/image.png");
    auto image_copy = Resource("/base/glow/test/image-copy.png");
    remove_source(image_copy);
    doesnt_throw([&]{
        test_image = image_res.ref();
    }, "Can load image through resource system");
    is(test_image->size, IVec{7, 5}, "Image size is correct");
    is((*test_image)[{5, 4}], uint32(0x2674dbff), "Image content is correct");
    is((*test_image)[{3, 3}], uint32(0x2674dbff), "Previous run didn't accidentally overwrite test image");
    (*test_image)[{3, 3}] = uint32(0x44556677);
    doesnt_throw([&]{
        rename(image_res, image_copy);
        save(image_copy);
    }, "Can save image through resource system");
    Image* copy = nullptr;
    doesnt_throw([&]{
        unload(image_copy);
        copy = image_copy.ref();
    }, "Can load previously saved image");
    is((*copy)[{5, 4}], uint32(0x2674dbff), "Copied image content 1");
    is((*copy)[{3, 3}], uint32(0x44556677), "Copied image content 2");
    done_testing();
});
#endif
