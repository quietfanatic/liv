#include "file-texture.h"

#include <SDL2/SDL_image.h>
#include "../glow/gl.h"

namespace glow {

FileTexture::FileTexture (String filename, uint32 target) : Texture(target) {
    static const bool init [[maybe_unused]] = []{
         // TODO: more formats
        auto flags = IMG_INIT_JPG | IMG_INIT_PNG;
        AA(IMG_Init(flags) == flags);
        return true;
    }();
    SDL_Surface* surf = AS(IMG_Load(filename.c_str()));
     // Translate SDL formats into OpenGL formats
     // TODO: select more efficient internal format
    GLenum internal_format;
    GLenum format;
    GLenum type;
     // I think these are correct wrt endianness
    switch (surf->format->format) {
        case SDL_PIXELFORMAT_RGB332:
            internal_format = GL_RGB8;
            format = GL_RGB;
            type = GL_UNSIGNED_BYTE_3_3_2;
            break;
        case SDL_PIXELFORMAT_RGB444:
            internal_format = GL_RGB8;
            format = GL_RGB;
            type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case SDL_PIXELFORMAT_RGB555:
            internal_format = GL_RGB8;
            format = GL_RGB;
            type = GL_UNSIGNED_SHORT_5_5_5_1;
            break;
        case SDL_PIXELFORMAT_BGR555:
            internal_format = GL_RGB8;
            format = GL_BGR;
            type = GL_UNSIGNED_SHORT_5_5_5_1;
            break;
        case SDL_PIXELFORMAT_ARGB4444:
            internal_format = GL_RGBA8;
            format = GL_BGRA;
            type = GL_UNSIGNED_SHORT_4_4_4_4_REV;
            break;
        case SDL_PIXELFORMAT_RGBA4444:
            internal_format = GL_RGBA8;
            format = GL_RGBA;
            type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case SDL_PIXELFORMAT_ABGR4444:
            internal_format = GL_RGBA8;
            format = GL_RGBA;
            type = GL_UNSIGNED_SHORT_4_4_4_4_REV;
            break;
        case SDL_PIXELFORMAT_BGRA4444:
            internal_format = GL_RGBA8;
            format = GL_BGRA;
            type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case SDL_PIXELFORMAT_ARGB1555:
            internal_format = GL_RGBA8;
            format = GL_BGRA;
            type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
            break;
        case SDL_PIXELFORMAT_RGBA5551:
            internal_format = GL_RGBA8;
            format = GL_RGBA;
            type = GL_UNSIGNED_SHORT_5_5_5_1;
            break;
        case SDL_PIXELFORMAT_ABGR1555:
            internal_format = GL_RGBA8;
            format = GL_RGBA;
            type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
            break;
        case SDL_PIXELFORMAT_BGRA5551:
            internal_format = GL_RGBA8;
            format = GL_BGRA;
            type = GL_UNSIGNED_SHORT_5_5_5_1;
            break;
        case SDL_PIXELFORMAT_RGB565:
            internal_format = GL_RGB8;
            format = GL_RGB;
            type = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case SDL_PIXELFORMAT_BGR565:
            internal_format = GL_RGB8;
            format = GL_BGR;
            type = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case SDL_PIXELFORMAT_RGB24:
            internal_format = GL_RGB8;
            format = GL_RGB;
            type = GL_UNSIGNED_BYTE;
            break;
        case SDL_PIXELFORMAT_BGR24:
            internal_format = GL_RGB8;
            format = GL_BGR;
            type = GL_UNSIGNED_BYTE;
            break;
        case SDL_PIXELFORMAT_RGB888:
            internal_format = GL_RGB8;
            format = GL_BGRA;
            type = GL_UNSIGNED_INT_8_8_8_8_REV;
            break;
        case SDL_PIXELFORMAT_RGBX8888:
            internal_format = GL_RGB8;
            format = GL_RGBA;
            type = GL_UNSIGNED_INT_8_8_8_8;
            break;
        case SDL_PIXELFORMAT_BGR888:
            internal_format = GL_RGB8;
            format = GL_RGBA;
            type = GL_UNSIGNED_INT_8_8_8_8_REV;
            break;
        case SDL_PIXELFORMAT_BGRX8888:
            internal_format = GL_RGB8;
            format = GL_BGRA;
            type = GL_UNSIGNED_INT_8_8_8_8;
            break;
        case SDL_PIXELFORMAT_ARGB8888:
            internal_format = GL_RGBA8;
            format = GL_BGRA;
            type = GL_UNSIGNED_INT_8_8_8_8_REV;
            break;
        case SDL_PIXELFORMAT_RGBA8888:
            internal_format = GL_RGBA8;
            format = GL_RGBA;
            type = GL_UNSIGNED_INT_8_8_8_8;
            break;
        case SDL_PIXELFORMAT_ABGR8888:
            internal_format = GL_RGBA8;
            format = GL_RGBA;
            type = GL_UNSIGNED_INT_8_8_8_8_REV;
            break;
        case SDL_PIXELFORMAT_BGRA8888:
            internal_format = GL_RGBA8;
            format = GL_BGRA;
            type = GL_UNSIGNED_INT_8_8_8_8;
            break;
        case SDL_PIXELFORMAT_ARGB2101010:
            internal_format = GL_RGBA8;
            format = GL_BGRA;
            type = GL_UNSIGNED_INT_2_10_10_10_REV;
            break;
        default: {
            uint32 sdl_format;
            if (SDL_ISPIXELFORMAT_ALPHA(surf->format->format)) {
                sdl_format = SDL_PIXELFORMAT_RGBA8888;
                internal_format = GL_RGBA8;
                format = GL_RGBA;
                type = GL_UNSIGNED_INT_8_8_8_8;
            }
            else {
                sdl_format = SDL_PIXELFORMAT_RGB24;
                internal_format = GL_RGB8;
                format = GL_RGB;
                type = GL_UNSIGNED_BYTE;
            }
             // Nontrivial format, so ask SDL to convert
            SDL_Surface* new_surf = AS(SDL_ConvertSurfaceFormat(surf, sdl_format, 0));
            SDL_FreeSurface(surf);
            surf = new_surf;
            break;
        }
    }
     // Now upload texture
    AA(surf->w > 0 && surf->h > 0);
    glBindTexture(target, id);
    glTexImage2D(
        target, 0, internal_format,
        surf->w, surf->h, 0, format, type, surf->pixels
    );
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    SDL_FreeSurface(surf);
}

FileTexture::~FileTexture () { }

} using namespace glow;

#ifndef TAP_DISABLE_TESTS
#include "../hacc/resource.h"
#include "../tap/tap.h"
#include "../wind/window.h"
#include "colors.h"

static tap::TestSet tests ("base/glow/file-texture", []{
    using namespace tap;
    using namespace geo;

    IVec test_size = {120, 120};
    wind::Window window {
        .title = "base/glow/texture test window",
        .size = test_size,  // TODO: enforce window size!
         // Window being the wrong size due to OS restrictions screws up this test
        .hidden = true
    };
    window.open();
    glow::init();

    FileTexture tex (hacc::file_resource_root() + "/base/glow/test/image.png");
    auto size = tex.size();
    is(size, IVec{7, 5}, "Created texture has correct size");

    std::vector<RGBA8> got_pixels (area(size));
    glGetTexImage(tex.target, 0, GL_RGBA, GL_UNSIGNED_BYTE, got_pixels.data());
    is(got_pixels[10], RGBA8(0x2674dbff), "Created texture has correct content");

    done_testing();
});

#endif
