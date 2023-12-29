#include "book.h"

#include "memory.h"

namespace liv {

} using namespace liv;

#ifndef TAP_DISABLE_TESTS
#include "../dirt/ayu/resources/resource.h"
#include "../dirt/glow/image.h"
#include "../dirt/tap/tap.h"

static tap::TestSet tests ("liv/book", []{
    using namespace tap;

    IVec size = {120, 120};

    App app;
    //app.hidden = true;
    app.settings->WindowSettings::size = size;
    Memory mem;
    Book book (
        &app, std::make_unique<BookSource>(
            app.settings, BookType::Misc, Slice<IRI>{
                IRI("res/liv/test/image.png", iri::program_location()),
                IRI("res/liv/test/image2.png", iri::program_location())
            }
        ),
        &mem
    );

    book.view.draw_if_needed();
    glow::Image img (size);
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.state.get_page_number(), 1, "Initial page is 1");
    is(img[{60, 60}], glow::RGBA8(0x2674dbff), "First page is correct");

    book.next();
    book.view.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.state.get_page_number(), 2, "Next page is 2");
    is(img[{60, 60}], glow::RGBA8(0x45942eff), "Second page is correct");

    book.next();
    is(book.state.get_page_number(), 2, "Can't go past last page");
    book.seek(10000);
    is(book.state.get_page_number(), 2, "Can't seek past last page");

    book.prev();
    book.view.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.state.get_page_number(), 1, "Go back to page 1");
    is(img[{60, 60}], glow::RGBA8(0x2674dbff), "Going back to first page works");

    book.prev();
    book.view.draw_if_needed();
    is(book.state.get_page_number(), 1, "Can't go before page 1");

    is(img[{0, 60}], glow::RGBA8(0x2674dbff), "Default to auto_zoom_mode = fit");
    book.auto_zoom_mode(ORIGINAL);
    book.view.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{0, 60}], glow::RGBA8(0x000000ff), "auto_zoom_mode = original");

    book.auto_zoom_mode(FIT);
    book.spread_count(2);
    book.state.set_page_number(1);
    is(book.state.spread_range, IRange{0, 2}, "Viewing two pages");
    is(book.state.visible_range(), IRange{0, 2}, "Two visible pages");
    book.view.draw_if_needed();
    is(book.view.spread->pages.size(), usize(2), "Spread has two pages");
    is(book.view.spread->pages[1].offset.x, 7, "Spread second page has correct offset");
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{20, 60}], glow::RGBA8(0x2674dbff), "Left side of spread is correct");
    is(img[{100, 60}], glow::RGBA8(0x45942eff), "Right side of spread is correct");
    is(img[{20, 30}], glow::RGBA8(0x000000ff), "Spread doesn't fill too much area");

    book.spread_direction(LEFT);
    book.view.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{20, 60}], glow::RGBA8(0x45942eff), "spread direction left (left)");
    is(img[{100, 60}], glow::RGBA8(0x2674dbff), "spread direction left (right)");
    is(img[{20, 30}], glow::RGBA8(0x000000ff), "Spread doesn't fill too much area");

    book.next();
    is(book.state.spread_range, IRange{1, 3}, "spread_range can go off the end");
    is(book.state.visible_range(), IRange{1, 2}, "visible_range cannot go off the end");

    done_testing();
});

#endif
