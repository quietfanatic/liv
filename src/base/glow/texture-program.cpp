#include "texture-program.h"

#include "../hacc/resource.h"
#include "gl.h"
#include "image.h"
#include "program.h"

namespace glow {

struct TextureProgram : Program {
    int u_screen_rect = -1;
    int u_tex_rect = -1;

    void Program_after_link () override {
        u_screen_rect = glGetUniformLocation(id, "u_screen_rect");
        u_tex_rect = glGetUniformLocation(id, "u_tex_rect");
        int u_tex = glGetUniformLocation(id, "u_tex");
        glUniform1i(u_tex, 0);
        AA(u_screen_rect != -1);
        AA(u_tex_rect != -1);
        AA(u_tex != -1);
    }
};

void draw_texture (const Texture& tex, const Rect& screen_rect, const Rect& tex_rect) {
    AA(!!tex);
    AA(tex.target == GL_TEXTURE_2D);

    static TextureProgram* program = hacc::Resource("/base/glow/texture-program.hacc")["program"][1];
    program->use();

    glUniform1fv(program->u_screen_rect, 4, &screen_rect.l);
    glUniform1fv(program->u_tex_rect, 4, &tex_rect.l);
    glBindTexture(GL_TEXTURE_2D, tex);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

} using namespace glow;

HACCABLE(glow::TextureProgram,
    delegate(base<Program>())
)

#ifndef TAP_DISABLE_TESTS
#include <vector>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
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
    RGBA8 fg = uint32(0x2674dbf0);

    is(tex->size(), IVec{7, 5}, "Created texture has correct size");

    Image tex_image (tex->source.size());
    glGetTexImage(tex->target, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_image.pixels);
    is(tex_image[{4, 3}], fg, "Created texture has correct content");

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
            diag(hacc::item_to_string(&expected[{x, y}], hacc::COMPACT));
            diag(hacc::item_to_string(&got[{x, y}], hacc::COMPACT));
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
//    SDL_GL_SwapWindow(window.sdl_window);
//    SDL_Delay(5000);

    done_testing();
});
#endif
