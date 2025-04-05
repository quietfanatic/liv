#include "page.h"

#include "../dirt/glow/program.h"
#include "../dirt/iri/path.h"
#include "../dirt/uni/io.h"
#include "../dirt/uni/time.h"
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
    if (texture) return;
    plog("Loading page");
    load_started_at = now();
    auto filename = iri::to_fs_path(location);
    try {
        texture = std::make_unique<FileTexture>(filename, GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
    load_finished_at = now();
    plog("loaded page");
}

void Page::unload () {
    texture = null;
    load_started_at = 0;
    load_finished_at = 0;
    load_failed = false;
}

 // These must match the constants in page.ayu#fragment
enum class Interpolator {
    Nearest = 0,
    Linear = 1,
    Cubic = 2,
    Lanczos16 = 3,
    Box9 = 5,
    Box16 = 6,
    Box25 = 7,
    Box36 = 8,
    Box49 = 9,
//    Box64 = 10,
};

struct PageProgram : Program {
    int u_orientation = -1;
    int u_screen_rect = -1;
    int u_tex_rect = -1;
    int u_interpolator = -1;
    int u_deringer = -1;
    int u_transparency_background = -1;
    int u_zoom = -1;
    int u_color_mul = -1;
    int u_color_add = -1;

    void Program_after_link () override {
        u_orientation = glGetUniformLocation(id, "u_orientation");
        expect(u_orientation != -1);
        u_screen_rect = glGetUniformLocation(id, "u_screen_rect");
        expect(u_screen_rect != -1);
        u_tex_rect = glGetUniformLocation(id, "u_tex_rect");
        expect(u_tex_rect != -1);
        int u_tex = glGetUniformLocation(id, "u_tex");
        expect(u_tex != -1);
        glUniform1i(u_tex, 0);
        u_interpolator = glGetUniformLocation(id, "u_interpolator");
        expect(u_interpolator != -1);
        u_deringer = glGetUniformLocation(id, "u_deringer");
        expect(u_deringer != -1);
        u_transparency_background = glGetUniformLocation(id, "u_transparency_background");
        expect(u_transparency_background != -1);
        u_zoom = glGetUniformLocation(id, "u_zoom");
        expect(u_zoom != -1);
        u_color_mul = glGetUniformLocation(id, "u_color_mul");
        expect(u_color_mul != -1);
        u_color_add = glGetUniformLocation(id, "u_color_add");
        expect(u_color_add != -1);
        plog("linked gl program");
    }
};

void draw_pages (
    Slice<PageView> views,
    const Settings& settings,
    Vec picture_size,
    Vec offset,
    float zoom
) {
    static PageProgram* program = ayu::track(
        program, "res:/liv/page.ayu#program"
    );
    program->use();
    double view_time = uni::now();

     // Shared parameters
    auto ori = settings.get(&LayoutSettings::orientation);
    glUniform1i(program->u_orientation, i32(ori));

    Interpolator interp;
    if (zoom == 1.f) {
         // Increase chances of pixel-perfect rendering
        interp = Interpolator::Nearest;
    }
    else if (zoom > 1.f) {
        auto upscaler = settings.get(&RenderSettings::upscaler);
        interp = Interpolator(i32(upscaler));
    }
    else {
        auto downscaler = settings.get(&RenderSettings::downscaler);
         // Don't use higher sample count than necessary.
        Downscaler necessary =
            zoom >= 1/2.f ? Downscaler::Box9
          : zoom >= 1/3.f ? Downscaler::Box16
          : zoom >= 1/4.f ? Downscaler::Box25
          : zoom >= 1/5.f ? Downscaler::Box36
          :                 Downscaler::Box49;
        if (i32(downscaler) > i32(necessary)) {
            downscaler = necessary;
        }
        interp = Interpolator(i32(downscaler));
    }
    glUniform1i(program->u_interpolator, i32(interp));

    auto deringer = settings.get(&RenderSettings::deringer);
    glUniform1i(program->u_deringer, i32(deringer));

    auto bg = settings.get(&RenderSettings::transparency_background);
    auto bg_scaled = Vec4(bg.r, bg.g, bg.b, bg.a) / 255.f;
    glUniform4fv(program->u_transparency_background, 1, &bg_scaled[0]);

    glUniform1f(program->u_zoom, zoom);

    auto& color = settings.get(&RenderSettings::color_range);
    auto color_mul = geo::size(color);
    auto color_add = color.l;
    glUniform3fv(program->u_color_mul, 1, &color_mul[0]);
    glUniform3fv(program->u_color_add, 1, &color_add[0]);

    for (auto& view : views) {
        auto texture = &*view.page->texture;
         // Some validation
        if (!texture) continue;  // Probably failed to load.
        expect(!!*texture);
        expect(texture->target == GL_TEXTURE_2D);
        plog("drawing page");

        view.page->last_viewed_at = view_time;
        Rect unzoomed = Rect(
            view.offset,
            view.offset + view.page->size
        );
        Rect zoomed = unzoomed * zoom + offset;
         // Convert to OpenGL coords (-1,-1)..(+1,+1)
        Rect on_picture = zoomed / picture_size * float(2) - Vec(1, 1);
        glUniform1fv(program->u_screen_rect, 4, &on_picture.l);

        auto tex_rect = Rect(Vec{0, 0}, view.page->size);
        glUniform1fv(program->u_tex_rect, 4, &tex_rect.l);
         // Do it
        glBindTexture(GL_TEXTURE_2D, *texture);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        plog("drew page");
    }
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
    settings.render.upscaler = Upscaler::Linear;
    settings.render.window_background = Fill::Black;

    UniqueArray<PageView> views {
        PageView{&page, Vec{0, 0}}
    };

    doesnt_throw([&]{
        draw_pages(views, settings, test_size, Vec{25, 35}, 10);
    }, "Page::draw");
    glFinish();

    UniqueImage expected (test_size);
    for (int y = 0; y < test_size.y; y++)
    for (int x = 0; x < test_size.x; x++) {
        if (y >= 35 && y < 85
         && x >= 25 && x < 95) {
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
            diag(cat(x, ' ', y));
            diag(ayu::item_to_string(&expected[{x, y}]));
            diag(ayu::item_to_string(&got[{x, y}]));
            goto done;
        }
    }
    done:;
    ok(match, "Page program wrote correct pixels");

     // TODO: test failure to load image
    done_testing();
});
#endif
