#include "texture-program.h"

#include "../hacc/resource.h"
#include "gl.h"
#include "image.h"
#include "program.h"

namespace glow {

struct TextureProgram : Program {
    int u_screen_rect = -1;
    int u_tex_rect = -1;
    int u_tex_target = -1;

    void Program_after_link () override {
        u_screen_rect = glGetUniformLocation(id, "u_screen_rect");
        u_tex_rect = glGetUniformLocation(id, "u_tex_rect");
        u_tex_target = glGetUniformLocation(id, "u_tex_target");
        for (auto name : {"u_2d_tex", "u_rectangle_tex", "u_1d_array_tex"}) {
            glUniform1i(glGetUniformLocation(id, name), 0);
        }
    }
};

void draw_texture (const Texture& tex, const Rect& screen_rect, const Rect& tex_rect) {
    static TextureProgram* program = hacc::Resource("/base/glow/texture-program.hacc")["program"][1];

    int tex_target = 0;
    switch (tex.target) {
        case GL_TEXTURE_2D: tex_target = 0; break;
        case GL_TEXTURE_RECTANGLE: tex_target = 1; break;
        case GL_TEXTURE_1D_ARRAY: tex_target = 2; break;
        default: AA(0); break;
    }
    program->use();
    glUniform1fv(program->u_screen_rect, 4, &screen_rect.l);
    glUniform1fv(program->u_tex_rect, 4, &tex_rect.l);
    glUniform1i(program->u_tex_target, tex_target);
    glBindTexture(tex.target, tex);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

} using namespace glow;

HACCABLE(glow::TextureProgram,
    delegate(base<Program>())
)

#ifndef TAP_DISABLE_TESTS
#include <vector>
#include "../hacc/serialize.h"
#include "../tap/tap.h"
#include "../wind/window.h"

static tap::TestSet tests ("base/glow/texture-program", []{
    using namespace tap;
    IVec test_size = {120, 120};
    wind::Window window {
        .title = "base/glow/texture test window",
        .size = test_size,  // TODO: enforce window size!
         // Window being the wrong size due to OS restrictions screws up this test
        .hidden = true
    };
    window.open();
    glow::init();

    ImageTexture* tex;
    doesnt_throw([&]{
        tex = hacc::Resource("/base/glow/test/texture-test.hacc")["texture"][1];
    }, "Can load texture");

    RGBA8 bg = uint32(0x331100ee);
    RGBA8 fg = uint32(0x2674dbff);

    int width; glGetTexLevelParameteriv(tex->target, 0, GL_TEXTURE_WIDTH, &width);
    is(width, 7, "Created texture has correct width");
    int height; glGetTexLevelParameteriv(tex->target, 0, GL_TEXTURE_HEIGHT, &height);
    is(height, 5, "Created texture has correct height");

    Image tex_image (tex->source.size());
    glGetTexImage(tex->target, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_image.pixels);
    is(tex_image[{1, 1}], fg, "Created texture has correct content");

    glClearColor(bg.r/255.f, bg.g/255.f, bg.b/255.f, bg.a/255.f);
    glClear(GL_COLOR_BUFFER_BIT);

    doesnt_throw([&]{
        draw_texture(*tex, Rect{-.5, -.5, .5, .5});
    }, "Can draw texture");

    Image expected (test_size);
    for (int y = 0; y < test_size.y; y++)
    for (int x = 0; x < test_size.x; x++) {
        if (y >= test_size.y / 4 && y < test_size.y * 3 / 4
         && x >= test_size.x / 4 && x < test_size.x * 3 / 4) {
            expected[{x, y}] = fg;
        }
        else {
            expected[{x, y}] = bg;
        }
    }

    Image got (test_size);
    glReadPixels(0, 0, test_size.x, test_size.y, GL_RGBA, GL_UNSIGNED_BYTE, got.pixels);

    bool match = true;
    for (int y = 0; y < test_size.y; y++)
    for (int x = 0; x < test_size.x; x++) {
        if (expected[{x, y}] != got[{x, y}]) {
            match = false;
            goto no_match;
        }
    }
    no_match:;
    if (!ok(match, "Texture program wrote correct pixels")) {
         // NOTE: these images will be upside-down.
         // TODO: bring image parsing/saving back
//        expected.save(hacc::resource_filename("/base/glow/test/texture-fail-expected"));
//        got.save(hacc::resource_filename("/base/glow/test/texture-fail-got"));
    }

    done_testing();
});
#endif
