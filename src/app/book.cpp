#include "book.h"

#include <filesystem>
#include <SDL2/SDL_video.h>
#include "../base/glow/gl.h"
#include "app.h"
#include "page.h"
#include "settings.h"

using namespace geo;
namespace fs = std::filesystem;

namespace app {

Book::Book (App& app, Str folder) :
    app(app),
    view(app.settings->default_view),
    folder(folder)
{
    AA(false);
}
Book::Book (App& app, const std::vector<String>& filenames) :
    app(app),
    view(app.settings->default_view),
    window{
        .title = "Image Viewer",
        .size = app.default_window_size,
        .resizable = true,
        .hidden = app.hidden
    }
{
    window.open();
    glow::init();
    pages.reserve(filenames.size());
    for (auto& filename : filenames) {
        pages.emplace_back(std::make_unique<Page>(filename));
    }
}
Book::~Book () { }

void Book::draw () {
     // Clear
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (valid_page_no(current_page_no)) {
        auto& page = *pages[current_page_no-1];
        Rect page_position = view.page_position(page.size, window.size);
         // Convert to OpenGL coords (-1,-1)..(+1,+1)
        Rect screen_rect = page_position / Vec(window.size) * float(2) - Vec(1, 1);
         // Draw
        page.draw(screen_rect);
    }
    SDL_GL_SwapWindow(window.sdl_window);
}

void Book::prev () {
    current_page_no -= 1;
    if (!valid_page_no(current_page_no)) {
        current_page_no = 1;
    }
    draw();
}

void Book::next () {
    current_page_no += 1;
    if (!valid_page_no(current_page_no)) {
        current_page_no = pages.size();
    }
    draw();
}

bool Book::valid_page_no (isize no) {
    return no >= 1 && usize(no) <= pages.size();
}

} using namespace app;

#ifndef TAP_DISABLE_TESTS
#include "../base/ayu/resource.h"
#include "../base/glow/image.h"
#include "../base/tap/tap.h"

static tap::TestSet tests ("app/book", []{
    using namespace tap;

    IVec size = {120, 120};

    App app;
    app.hidden = true;
    app.default_window_size = size;
    Book book (app, {
        ayu::file_resource_root() + "/base/glow/test/image.png"sv,
        ayu::file_resource_root() + "/base/glow/test/image2.png"sv
    });

    book.draw();
    glow::Image img (size);
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.current_page_no, 1, "Initial page is 1");
    is(img[{60, 60}], glow::RGBA8(0x2674dbff), "First page is correct");

    book.next();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.current_page_no, 2, "Next page is 2");
    is(img[{60, 60}], glow::RGBA8(0x45942eff), "Second page is correct");

    book.next();
    is(book.current_page_no, 2, "Can't go past last page");

    book.prev();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.current_page_no, 1, "Go back to page 1");
    is(img[{60, 60}], glow::RGBA8(0x2674dbff), "Going back to first page works");

    book.prev();
    is(book.current_page_no, 1, "Can't go before page 1");

    is(img[{0, 0}], glow::RGBA8(0x00000000), "Default to fit mode");
    book.view.fit_mode = STRETCH;
    book.draw();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{0, 0}], glow::RGBA8(0x2674dbff), "Stretch mode fills screen");

    done_testing();
});

#endif
