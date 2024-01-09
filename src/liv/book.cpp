#include "book.h"

#include <SDL2/SDL_events.h>
#include "memory.h"

namespace liv {

void Book::on_event (SDL_Event* e) {
    switch (e->type) {
        case SDL_WINDOWEVENT: {
            switch (e->window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED: {
                    view.window_size_changed({
                        e->window.data1,
                        e->window.data2
                    });
                    break;
                }
                case SDL_WINDOWEVENT_EXPOSED: {
                    view.need_draw = true;
                    break;
                }
                case SDL_WINDOWEVENT_FOCUS_GAINED: {
                    if (!state.settings->get(
                        &WindowSettings::automated_input
                    )) {
                        last_focused = e->window.timestamp;
                    }
                    break;
                }
            }
            break;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
             // There's a bug where if we gain focus by another window being
             // closed with a keystroke, the keystroke gets sent to us.  I don't
             // know if this is a bug in SDL or the window manager, but the
             // workaround is pretty simple: disable keyboard input right after
             // gaining focus.  In my testing, the difference is always either 0
             // or 1 ms, so we'll go up to 3 in case the computer is slow for
             // some reason.  This is still faster than 1 video frame and faster
             // than the typical input device polling rate (10ms).
            if (e->key.timestamp - last_focused <= 3) return;
            else break;
        }
        case SDL_MOUSEMOTION: {
            if (e->motion.state & SDL_BUTTON_RMASK) {
                drag(geo::Vec(
                    e->motion.xrel,
                    e->motion.yrel
                ) * state.settings->get(&ControlSettings::drag_speed));
            }
            break;
        }
         // TODO: Support wheel
        default: break;
    }
    auto action = state.settings->map_event(e);
    if (action && *action) (*action)();
}

 // Commands
void Book::fullscreen () {
    view.set_fullscreen(!view.is_fullscreen());
    view.layout = {};
    view.need_draw = true;
}

void Book::set_page_offset (int32 off) {
    auto spread_count = state.settings->get(&LayoutSettings::spread_count);
     // Clamp such that there is at least one visible page in the range
    state.page_offset = clamp(
        off,
        1 - int32(spread_count),
        int32(block.pages.size()) - 1
    );
    if (state.settings->get(&LayoutSettings::reset_zoom_on_page_turn)) {
        state.manual_zoom = GNAN;
        state.manual_offset = GNAN;
    }
}

void Book::next () {
    seek(state.settings->get(&LayoutSettings::spread_count));
    need_memorize = true;
}

void Book::prev () {
    seek(-state.settings->get(&LayoutSettings::spread_count));
    need_memorize = true;
}

void Book::seek (int32 offset) {
    set_page_offset(state.page_offset + offset);
    view.spread = {};
    view.layout = {};
    view.need_draw = true;
    need_memorize = true;
}

void Book::remove_current_page () {
    auto visible = visible_range();
    if (!size(visible)) return;
    block.unload_page(block.get(visible.l));
    block.pages.erase(visible.l);
     // Reclamp page offset
    set_page_offset(state.page_offset);
    view.spread = {};
    view.layout = {};
    view.need_draw = true;
    need_memorize = true;
}

void Book::sort (SortMethod method) {
    auto visible = visible_range();
    IRI current_location = size(visible)
        ? block.pages[visible.l]->location
        : IRI();
    block.resort(method);
    if (current_location) {
        for (usize i = 0; i < size(block.pages); i++) {
            if (block.pages[i]->location == current_location) {
                set_page_offset(i);
                break;
            }
        }
    }
    view.spread = {};
    view.layout = {};
    view.need_draw = true;
    need_memorize = true;
}

void Book::spread_count (int32 count) {
    state.settings->layout.spread_count = {clamp(count, 1, 2048)};
    view.spread = {};
    view.layout = {};
    view.need_draw = true;
    need_memorize = true;
}

void Book::spread_direction (Direction dir) {
    state.settings->layout.spread_direction = {dir};
    view.spread = {};
    view.layout = {};
    view.need_draw = true;
    need_memorize = true;
}

void Book::auto_zoom_mode (AutoZoomMode mode) {
    state.set_auto_zoom_mode(mode);
    view.layout = {};
    view.need_draw = true;
    need_memorize = true;
}

void Book::align (Vec small, Vec large) {
    state.set_align(small, large);
    view.spread = {};
    view.layout = {};
    view.need_draw = true;
    need_memorize = true;
}

void Book::zoom_multiply (float factor) {
     // Need spread to clamp the zoom
    auto& spread = view.get_spread();
     // Actually we also need the layout to multiply the zoom
    auto& layout = view.get_layout();
     // Set manual zoom
    state.manual_zoom = spread.clamp_zoom(
        *state.settings, layout.zoom * factor
    );
    if (defined(state.manual_offset)) {
         // Hacky way to zoom from center
         // TODO: zoom to preserve current alignment instead
        state.manual_offset +=
            spread.size * (layout.zoom - state.manual_zoom) / 2;
    }
    view.layout = {};
    view.need_draw = true;
    need_memorize = true;
}

void Book::reset_layout () {
    state.reset_layout();
    view.spread = {};
    view.layout = {};
    view.need_draw = true;
    need_memorize = true;
}

void Book::reset_settings () {
     // Preserve the parent
    auto old_sort = state.settings->get(&FilesSettings::sort);
    auto parent = state.settings->parent;
    *state.settings = {};
    state.settings->parent = parent;
    state.manual_zoom = GNAN;
    state.manual_offset = GNAN;
    auto new_sort = state.settings->get(&FilesSettings::sort);
    if (new_sort != old_sort) block.resort(new_sort);
    view.spread = {};
    view.layout = {};
    view.need_draw = true;
    need_memorize = true;
}

void Book::interpolation_mode (InterpolationMode mode) {
    state.settings->render.interpolation_mode = mode;
    view.need_draw = true;
    need_memorize = true;
}

void Book::window_background (Fill bg) {
    state.settings->render.window_background = bg;
    view.need_draw = true;
    need_memorize = true;
}

void Book::transparency_background (Fill bg) {
    state.settings->render.transparency_background = bg;
    view.need_draw = true;
    need_memorize = true;
}

 // Not a command, but we need to figure out how to make this configurable.
void Book::drag (Vec amount) {
    if (!defined(state.manual_offset)) {
         // Transition from automatic to manual
        auto& layout = view.get_layout();
        state.manual_offset = layout.offset;
        state.manual_zoom = layout.zoom;
    }
    state.manual_offset += amount;
    view.layout = {};
    view.need_draw = true;
    need_memorize = true;
}

} using namespace liv;

#ifndef TAP_DISABLE_TESTS
#include "../dirt/ayu/resources/resource.h"
#include "../dirt/glow/image.h"
#include "../dirt/tap/tap.h"

static tap::TestSet tests ("liv/book", []{
    using namespace tap;

    auto size = IVec{120, 120};

    auto settings = std::make_unique<Settings>();
    settings->window.size = {size};
    settings->parent = app_settings();
    auto src = std::make_unique<BookSource>(
        BookType::Misc, Slice<IRI>{
            IRI("res/liv/test/image.png", iri::program_location()),
            IRI("res/liv/test/image2.png", iri::program_location())
        }
    );
    Book book (move(src), BookState(move(settings)));

    book.view.draw_if_needed();
    glow::Image img (size);
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.state.page_offset, 0, "Initial page is 1");
    is(img[{60, 60}], glow::RGBA8(0x2674dbff), "First page is correct");

    book.next();
    book.view.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.state.page_offset, 1, "Next page is 2");
    is(img[{60, 60}], glow::RGBA8(0x45942eff), "Second page is correct");

    book.next();
    is(book.state.page_offset, 1, "Can't go past last page");
    book.seek(10000);
    is(book.state.page_offset, 1, "Can't seek past last page");

    book.prev();
    book.view.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(book.state.page_offset, 0, "Go back to page 1");
    is(img[{60, 60}], glow::RGBA8(0x2674dbff), "Going back to first page works");

    book.prev();
    book.view.draw_if_needed();
    is(book.state.page_offset, 0, "Can't go before page 1");

    is(img[{0, 60}], glow::RGBA8(0x2674dbff), "Default to auto_zoom_mode = fit");
    book.auto_zoom_mode(AutoZoomMode::Original);
    book.view.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{0, 60}], glow::RGBA8(0x000000ff), "auto_zoom_mode = original");

    book.auto_zoom_mode(AutoZoomMode::Fit);
    book.spread_count(2);
    book.set_page_offset(0);
    is(book.visible_range(), IRange{0, 2}, "Two visible pages");
    book.view.draw_if_needed();
    is(book.view.spread->pages.size(), usize(2), "Spread has two pages");
    is(book.view.spread->pages[1].offset.x, 7, "Spread second page has correct offset");
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{20, 60}], glow::RGBA8(0x2674dbff), "Left side of spread is correct");
    is(img[{100, 60}], glow::RGBA8(0x45942eff), "Right side of spread is correct");
    is(img[{20, 30}], glow::RGBA8(0x000000ff), "Spread doesn't fill too much area");

    book.spread_direction(Direction::Left);
    book.view.draw_if_needed();
    glFinish();
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);
    is(img[{20, 60}], glow::RGBA8(0x45942eff), "spread direction left (left)");
    is(img[{100, 60}], glow::RGBA8(0x2674dbff), "spread direction left (right)");
    is(img[{20, 30}], glow::RGBA8(0x000000ff), "Spread doesn't fill too much area");

    book.next();
    is(book.visible_range(), IRange{1, 2}, "visible_range cannot go off the end");

    done_testing();
});

#endif
