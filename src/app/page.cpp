#include "page.h"

#include "../base/glow/program.h"
#include "../base/ayu/describe-standard.h"
#include "../base/ayu/resource.h"

using namespace geo;
using namespace glow;

namespace app {

Page::Page (String filename) :
    filename(filename)
{ }
Page::~Page () { }

void Page::load () {
    if (!texture) {
        texture = std::make_unique<FileTexture>(filename, GL_TEXTURE_RECTANGLE);
        size = texture->size();
        estimated_memory = area(size) * ((texture->bpp() + 1) / 8);
    }
}

void Page::unload () {
    texture = {};
}

struct PageProgram : Program {
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

void Page::draw (const Rect& screen_rect, const Rect& tex_rect) {
    AA(texture && *texture);
    AA(texture->target == GL_TEXTURE_RECTANGLE);

    static PageProgram* program = ayu::Resource("/app/page.ayu")["program"][1];
    program->use();

    glUniform1fv(program->u_screen_rect, 4, &screen_rect.l);
    if (defined(tex_rect)) {
        glUniform1fv(program->u_tex_rect, 4, &tex_rect.l);
    }
    else {
        auto whole_page = Rect(Vec{0, 0}, size);
        glUniform1fv(program->u_tex_rect, 4, &whole_page.l);
    }
    glBindTexture(GL_TEXTURE_RECTANGLE, *texture);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

} using namespace app;

AYU_DESCRIBE(app::PageProgram,
    delegate(base<Program>())
)

#ifndef TAP_DISABLE_TESTS
#include "../base/glow/image.h"
#include "../base/ayu/serialize.h"
#include "../base/tap/tap.h"
#include "../base/wind/window.h"

static tap::TestSet tests ("app/page", []{
    using namespace tap;
    IVec test_size = {120, 120};
    wind::Window window {
        .title = "base/glow/texture test window",
        .size = test_size,  // TODO: enforce window size!
         // Window being the wrong size due to OS restrictions screws up this test
        .hidden = true
    };
    window.open();
    init();

    Page page (ayu::file_resource_root() + "/base/glow/test/image.png");
    is(page.size, IVec(0, 0), "Page isn't loaded yet");
    page.load();
    is(page.size, IVec(7, 5), "Page has correct size");

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    doesnt_throw([&]{
        page.draw(Rect(-.5, -.5, .5, .5));
    }, "Page::draw");

    Image expected (test_size);
    for (int y = 0; y < test_size.y; y++)
    for (int x = 0; x < test_size.x; x++) {
        if (y >= test_size.y / 4 && y < test_size.y * 3 / 4
         && x >= test_size.x / 4 && x < test_size.x * 3 / 4) {
            expected[{x, y}] = RGBA8(0x2674dbff);
        }
        else {
            expected[{x, y}] = RGBA8(0, 0, 0, 0);
        }
    }

    Image got (test_size);
    glReadPixels(0, 0, test_size.x, test_size.y, GL_RGBA, GL_UNSIGNED_BYTE, got.pixels);

    bool match = true;
    for (int y = 0; y < test_size.y; y++)
    for (int x = 0; x < test_size.x; x++) {
        if (expected[{x, y}] != got[{x, y}]) {
            match = false;
            diag(ayu::item_to_string(&expected[{x, y}], ayu::COMPACT));
            diag(ayu::item_to_string(&got[{x, y}], ayu::COMPACT));
            goto no_match;
        }
    }
    no_match:;
    ok(match, "Page program wrote correct pixels");

    done_testing();
});
#endif
