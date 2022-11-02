#include "file-texture.h"

#include <SDL2/SDL_image.h>
#include "../glow/gl.h"

#ifdef GLOW_PROFILING
#include "../uni/time.h"
#endif

namespace glow {

FileTexture::FileTexture (String filename, uint32 target) : Texture(target) {
    static const bool init [[maybe_unused]] = []{
         // TODO: more formats
        auto flags = IMG_INIT_JPG | IMG_INIT_PNG;
        AA(IMG_Init(flags) == flags);
        return true;
    }();
#ifdef GLOW_PROFILING
    double time0 = uni::now();
#endif
    SDL_Surface* surf = AS(IMG_Load(filename.c_str()));
#ifdef GLOW_PROFILING
    double time1 = uni::now();
    double time2 = time1;
#endif
     // Translate SDL formats into OpenGL formats
     // TODO: select more efficient internal format
    GLenum internal_format;
    GLenum format;
    GLenum type;
     // I think these are correct wrt endianness
    switch (surf->format->format) {
         // I've never seen IMG_Load give me image formats besides these, so
         // we'll go ahead and convert everything else
        case SDL_PIXELFORMAT_RGB24:
            internal_format = GL_RGB8;
            format = GL_RGB;
            type = GL_UNSIGNED_BYTE;
            break;
        case SDL_PIXELFORMAT_RGBA32:
            internal_format = GL_RGBA8;
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;
        default: {
            uint32 sdl_format;
            if (SDL_ISPIXELFORMAT_ALPHA(surf->format->format)) {
                sdl_format = SDL_PIXELFORMAT_RGBA32;
                internal_format = GL_RGBA8;
                format = GL_RGBA;
                type = GL_UNSIGNED_BYTE;
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
#ifdef GLOW_PROFILING
            time2 = uni::now();
#endif
            break;
        }
    }
     // Detect greyscale images and unused alpha channels
    auto pixels = reinterpret_cast<uint8*>(surf->pixels);
    if (surf->format->format == SDL_PIXELFORMAT_RGB24) {
        bool greyscale = true;
        for (int32 y = 0; y < surf->h; y++)
        for (int32 x = 0; x < surf->w; x++) {
            uint8 r = pixels[y * surf->pitch + x*3];
            uint8 g = pixels[y * surf->pitch + x*3 + 1];
            uint8 b = pixels[y * surf->pitch + x*3 + 2];
            if (r != g || g != b) {
                greyscale = false;
                goto done_24;
            }
        }
        done_24:;
        if (greyscale) internal_format = GL_R8;
    }
    else if (surf->format->format == SDL_PIXELFORMAT_RGBA32) {
        bool greyscale = true;
        bool unused_alpha = true;
        for (int32 y = 0; y < surf->h; y++)
        for (int32 x = 0; x < surf->w; x++) {
            uint8 r = pixels[y * surf->pitch + x*4];
            uint8 g = pixels[y * surf->pitch + x*4 + 1];
            uint8 b = pixels[y * surf->pitch + x*4 + 2];
            uint8 a = pixels[y * surf->pitch + x*4 + 3];
            if (r != g || g != b) {
                greyscale = false;
                goto done_32;
            }
            if (a != 255) {
                unused_alpha = false;
                goto done_32;
            }
        }
        done_32:;
        if (greyscale && unused_alpha) internal_format = GL_R8;
        else if (greyscale) internal_format = GL_RG8; // G -> A
        else if (unused_alpha) internal_format = GL_RGB8;
    }
#ifdef GLOW_PROFILING
    double time3 = uni::now();
#endif
     // Now upload texture
    AA(surf->w > 0 && surf->h > 0);
    glBindTexture(target, id);
    glTexImage2D(
        target, 0, internal_format,
        surf->w, surf->h, 0, format, type, surf->pixels
    );
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     // Make sure the right channels go to the right colors
    if (internal_format == GL_R8) {
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_RED);
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_RED);
    }
    else if (internal_format == GL_RG8) {
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_RED);
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_RED);
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_GREEN);
    }
#ifdef GLOW_PROFILING
    double time4 = uni::now();
    int64 mem = surf->w * surf->h;
    if (internal_format == GL_RG8) mem *= 2;
    else if (internal_format == GL_RGB8) mem *= 3;
    else if (internal_format == GL_RGBA8) mem *= 4;
    ayu::dump(filename, time1 - time0, time2 - time1, time3 - time2, time4 - time3, mem);
#endif
    SDL_FreeSurface(surf);
}

FileTexture::~FileTexture () { }

} using namespace glow;

#ifndef TAP_DISABLE_TESTS
#include "../ayu/resource.h"
#include "../tap/tap.h"
#include "colors.h"
#include "test-environment.h"

static tap::TestSet tests ("base/glow/file-texture", []{
    using namespace tap;
    using namespace geo;

    TestEnvironment env;

    FileTexture tex (ayu::file_resource_root() + "/base/glow/test/image.png");
    auto size = tex.size();
    is(size, IVec{7, 5}, "Created texture has correct size");

    std::vector<RGBA8> got_pixels (area(size));
    glGetTexImage(tex.target, 0, GL_RGBA, GL_UNSIGNED_BYTE, got_pixels.data());
    is(got_pixels[10], RGBA8(0x2674dbff), "Created texture has correct content");

    done_testing();
});

#endif
