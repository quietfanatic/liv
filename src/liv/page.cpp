#include "page.h"

#include "../dirt/glow/program.h"
#include "../dirt/iri/path.h"
#include "../dirt/uni/io.h"
#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/resources/resource.h"
#include "app.h"
#include "book.h"

using namespace glow;

namespace liv {

Page::Page (const IRI& loc) :
    location(loc)
{ }
Page::~Page () { }

void Page::load () {
    plog("Loading page");
    if (texture) return;
    auto filename = iri::to_fs_path(location);
    try {
        texture = std::make_unique<FileTexture>(filename, GL_TEXTURE_RECTANGLE);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        size = texture->size();
        estimated_memory = area(size) * ((texture->bpp() + 1) / 8);
    }
    catch (std::exception& e) {
        ayu::warn_utf8(cat(
            "Error loading image file ", filename,
            ": ", e.what(), "\n"
        ));
        load_failed = true;
    }
    plog("loaded page");
}

void Page::unload () {
    texture = null;
    load_failed = false;
}

struct PageProgram : Program {
    int u_screen_rect = -1;
    int u_tex_rect = -1;
    int u_interpolation_mode = -1;
    int u_transparency_background = -1;
    int u_zoom = -1;

    void Program_after_link () override {
        u_screen_rect = glGetUniformLocation(id, "u_screen_rect");
        u_tex_rect = glGetUniformLocation(id, "u_tex_rect");
        int u_tex = glGetUniformLocation(id, "u_tex");
        glUniform1i(u_tex, 0);
        u_interpolation_mode = glGetUniformLocation(id, "u_interpolation_mode");
        u_transparency_background = glGetUniformLocation(id, "u_transparency_background");
        u_zoom = glGetUniformLocation(id, "u_zoom");
        expect(u_screen_rect != -1);
        expect(u_tex_rect != -1);
        expect(u_tex != -1);
        expect(u_interpolation_mode != -1);
        expect(u_transparency_background != -1);
        expect(u_zoom != -1);
        plog("linked gl program");
    }
};

void Page::draw (
    const Settings& settings,
    float zoom,
    const Rect& screen_rect,
    const Rect& tex_rect
) {
    plog("drawing page");
    if (!texture) return;
    require(!!*texture);
    require(texture->target == GL_TEXTURE_RECTANGLE);

    static constexpr IRI program_location = IRI("res:/liv/page.ayu");
    static PageProgram* program = ayu::ResourceRef(program_location)["program"][1];
    program->use();

    glUniform1fv(program->u_screen_rect, 4, &screen_rect.l);
    if (defined(tex_rect)) {
        glUniform1fv(program->u_tex_rect, 4, &tex_rect.l);
    }
    else {
        auto whole_page = Rect(Vec{0, 0}, size);
        glUniform1fv(program->u_tex_rect, 4, &whole_page.l);
    }
    auto interp = settings.get(&RenderSettings::interpolation_mode);
    glUniform1i(program->u_interpolation_mode, uint8(interp));
    auto bg = settings.get(&RenderSettings::transparency_background);
    glUniform4f(program->u_transparency_background,
        bg.r / 255.f, bg.g / 255.f, bg.b / 255.f, bg.a / 255.f
    );
    glUniform1f(program->u_zoom, zoom);
    glBindTexture(GL_TEXTURE_RECTANGLE, *texture);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    plog("drew page");
}

} using namespace liv;

AYU_DESCRIBE(liv::PageProgram,
#ifdef LIV_PROFILE
    swizzle([](PageProgram&, const ayu::Tree&){
        plog("loading program");
    }),
#endif
    delegate(base<Program>())
)

#ifndef TAP_DISABLE_TESTS
#include <SDL2/SDL.h>
#include "../dirt/glow/image.h"
#include "../dirt/ayu/traversal/to-tree.h"
#include "../dirt/tap/tap.h"
#include "../dirt/wind/window.h"

static tap::TestSet tests ("liv/page", []{
    using namespace tap;

    IVec test_size = {120, 120};
    wind::Window window (
        "Test window",
         // TODO: enforce window size!  Window being the wrong size due to OS
         // restrictions screws up this test
        test_size
    );
    SDL_MinimizeWindow(window);
    SDL_ShowWindow(window);
    SDL_MinimizeWindow(window);
    glow::init();

    Page page (IRI("res/liv/test/image.png", iri::program_location()));
    is(page.size, IVec(0, 0), "Page isn't loaded yet");
    page.load();
    is(page.size, IVec(7, 5), "Page has correct size");

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    Settings settings;
    settings.render.interpolation_mode = InterpolationMode::Linear;
    settings.render.window_background = Fill::Black;

    doesnt_throw([&]{
        page.draw(settings, 1, Rect(-.5, -.5, .5, .5));
    }, "Page::draw");

    UniqueImage expected (test_size);
    for (int y = 0; y < test_size.y; y++)
    for (int x = 0; x < test_size.x; x++) {
        if (y >= test_size.y / 4 && y < test_size.y * 3 / 4
         && x >= test_size.x / 4 && x < test_size.x * 3 / 4) {
            expected[{x, y}] = RGBA8(0x2674dbff);
        }
        else {
            expected[{x, y}] = RGBA8(0, 0, 0, 255);
        }
    }

    UniqueImage got (test_size);
    glReadPixels(0, 0, test_size.x, test_size.y, GL_RGBA, GL_UNSIGNED_BYTE, got.pixels);

    bool match = true;
    for (int y = 0; y < test_size.y; y++)
    for (int x = 0; x < test_size.x; x++) {
        if (expected[{x, y}] != got[{x, y}]) {
            match = false;
            diag(ayu::item_to_string(&expected[{x, y}]));
            diag(ayu::item_to_string(&got[{x, y}]));
            goto no_match;
        }
    }
    no_match:;
    ok(match, "Page program wrote correct pixels");

     // TODO: test failure to load image

    done_testing();
});
#endif
