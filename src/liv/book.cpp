#include "book.h"

#include <SDL2/SDL_events.h>
#include "../dirt/control/input.h"
#include "mark.h"

namespace liv {

Book::Book (
    BookSource&& src,
    PageBlock&& bl,
    BookState&& st
) :
    source(move(src)),
    block(move(bl)),
    state(move(st)),
    view(this)
{
     // If we were opened with one page, there's a good chance we'll be closed
     // without looking at any other pages, so don't bother preloading any
     // other images until we navigate once.
    if (source.type == BookType::FileWithNeighbors) {
        delay_preload = true;
    }
}

 // Some really awkward double delegation to make sure settings isn't used after
 // being moved from.
Book::Book (
    BookSource&& src,
    PageBlock&& bl,
    std::unique_ptr<Settings>&& settings
) :
    Book(move(src), move(bl), BookState(move(settings)))
{ }

Book::Book (
    BookSource&& src,
    std::unique_ptr<Settings> settings
) :
    Book(move(src), PageBlock(src, *settings), move(settings))
{ }

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
                    view.update_picture();
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
                scroll(geo::Vec(
                    e->motion.xrel,
                    e->motion.yrel
                ) * state.settings->get(&ControlSettings::drag_speed));
            }
            break;
        }
        case SDL_MOUSEWHEEL: {
            auto amount = Vec(e->wheel.preciseX, e->wheel.preciseY);
            amount.x = -amount.x;
            if (e->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
                amount.y = -amount.y;
            }
            amount *= state.settings->get(&ControlSettings::scroll_speed);
            scroll(amount);
            break;
        }
        default: break;
    }
    if (auto input = control::input_from_event(e)) {
        auto action = state.settings->map_input(input);
        if (action && *action) (*action)();
    }
}

void Book::set_page_offset (i32 off) {
    auto spread_count = state.settings->get(&LayoutSettings::spread_count);
     // Clamp such that there is at least one visible page in the range
    state.page_offset = clamp(
        off,
        1 - i32(spread_count),
        i32(block.pages.size()) - 1
    );
    switch (state.settings->get(&LayoutSettings::reset_on_seek)) {
        case ResetOnSeek::Zoom:
            state.manual_zoom = {};
            view.need_zoom = true;
            [[fallthrough]];
        case ResetOnSeek::Offset:
            state.manual_offset = {};
            view.need_offset = true;
            [[fallthrough]];
        case ResetOnSeek::None: break;
        default: never();
    }
}

void Book::next () {
    seek(state.settings->get(&LayoutSettings::spread_count));
}

void Book::prev () {
    seek(-state.settings->get(&LayoutSettings::spread_count));
}

void Book::seek (i32 offset) {
    set_page_offset(state.page_offset + offset);
    view.update_spread();
    need_mark = true;
    delay_preload = false;
}

void Book::go_next (Direction dir) {
    auto spread_dir = state.settings->get(&LayoutSettings::spread_direction);
    if (dir == spread_dir) next();
    else if (dir == -spread_dir) prev();
}

void Book::go (Direction dir, i32 offset) {
    auto spread_dir = state.settings->get(&LayoutSettings::spread_direction);
    if (dir == spread_dir) seek(offset);
    else if (dir == -spread_dir) seek(-offset);
}

void Book::remove_current_page () {
    auto visible = visible_range();
    if (!size(visible)) return;
    block.unload_page(block.get(visible.l));
    block.pages.erase(visible.l);
     // Reclamp page offset
    set_page_offset(state.page_offset);
    view.update_spread();
    need_mark = true;
    delay_preload = false;
}

void Book::sort (SortMethod method) {
    auto visible = visible_range();
    IRI current_location = size(visible)
        ? block.pages[visible.l]->location
        : IRI();
    block.resort(method);
    if (current_location) {
        for (u32 i = 0; i < size(block.pages); i++) {
            if (block.pages[i]->location == current_location) {
                set_page_offset(i);
                break;
            }
        }
    }
    view.update_spread();
    need_mark = true;
    delay_preload = false;
}

void Book::spread_count (i32 count) {
    state.settings->layout.spread_count = {
        clamp(count, 1, LayoutSettings::max_spread_count)
    };
    view.update_spread();
    need_mark = true;
    delay_preload = false;
}

void Book::spread_direction (Direction dir) {
    state.settings->layout.spread_direction = {dir};
    view.update_spread();
    need_mark = true;
}

void Book::auto_zoom_mode (AutoZoomMode mode) {
    state.settings->layout.auto_zoom_mode = {mode};
    state.manual_zoom = {};
    state.manual_offset = {};
    view.update_zoom();
    view.update_offset();
    need_mark = true;
}

void Book::set_zoom (float zoom) {
    state.manual_zoom = view.clamp_zoom(zoom);
    if (state.manual_offset) {
         // Hacky way to zoom from center.  TODO: make view.offset depend on
         // alignment
        Vec ss = view.get_spread_size();
        float zoom = view.get_zoom();
        *state.manual_offset += ss * (zoom - *state.manual_zoom) / 2;
    }
    view.update_zoom();
    view.update_offset(); // TODO see above
    need_mark = true;
}

void Book::zoom (float factor) {
    set_zoom(view.get_zoom() * factor);
}

void Book::align (Vec small, Vec large) {
    auto small_align = state.settings->get(&LayoutSettings::small_align);
    auto large_align = state.settings->get(&LayoutSettings::large_align);
    if (defined(small.x)) small_align.x = small.x;
    if (defined(small.y)) small_align.y = small.y;
    if (defined(large.x)) large_align.x = large.x;
    if (defined(large.y)) large_align.y = large.y;
    state.settings->layout.small_align = {small_align};
    state.settings->layout.large_align = {large_align};
    state.manual_offset = {};
     // Alignment affects spread, not just offset
    view.update_spread();
    need_mark = true;
}

void Book::orientation (Direction o) {
    state.settings->layout.orientation = o;
    view.update_picture_size();
    need_mark = true;
}

void Book::reset_layout () {
    auto sc = state.settings->layout.spread_count;
    state.settings->layout = {};
    state.settings->layout.spread_count = sc;
    state.manual_zoom = {};
    state.manual_offset = {};
    view.update_spread();
    need_mark = true;
}

void Book::reset_settings () {
    auto old_sort = state.settings->get(&FilesSettings::sort);
     // Preserve the parent
    auto parent = state.settings->parent;
    *state.settings = {};
    state.settings->parent = parent;
    state.manual_zoom = {};
    state.manual_offset = {};
     // Resort if sort has changed
    auto new_sort = state.settings->get(&FilesSettings::sort);
    if (new_sort != old_sort) block.resort(new_sort);
    view.update_picture_size();
    view.update_spread();
    need_mark = true;
}

void Book::interpolation_mode (InterpolationMode mode) {
    state.settings->render.interpolation_mode = mode;
    view.update_picture();
    need_mark = true;
}

void Book::window_background (Fill bg) {
    state.settings->render.window_background = bg;
    view.update_picture();
    need_mark = true;
}

void Book::transparency_background (Fill bg) {
    state.settings->render.transparency_background = bg;
    view.update_picture();
    need_mark = true;
}

void Book::color_range (const ColorRange& range) {
    state.settings->render.color_range = range;
    view.update_picture();
    need_mark = true;
}

void Book::scroll (Vec amount) {
    state.manual_zoom = {view.get_zoom()};
    state.manual_offset = {view.clamp_offset(view.get_offset() + amount)};
    view.update_zoom();
    view.update_offset();
    need_mark = true;
}

bool Book::idle_processing (const App& app) {
    if (need_mark) {
        need_mark = false;
        save_mark(app, *this);
        return true;
    }
    if (delay_preload) return false;
    return block.idle_processing(this, *state.settings);
}

} using namespace liv;

#ifndef TAP_DISABLE_TESTS
#include "../dirt/ayu/resources/resource.h"
#include "../dirt/glow/image.h"
#include "../dirt/tap/tap.h"

static tap::TestSet tests ("liv/book", []{
    using namespace tap;

    auto size = IVec{120, 120};

    App app;

    auto settings = std::make_unique<Settings>();
    settings->window.size = {size};
    settings->parent = app.app_settings;
    auto src = BookSource(
        BookType::Misc, Slice<IRI>{
            IRI("res/liv/test/image.png", iri::program_location()),
            IRI("res/liv/test/image2.png", iri::program_location())
        }
    );
    PageBlock block (src, *settings);
    BookState state (move(settings));
    Book book (move(src), move(block), move(state));

    book.view.draw_if_needed();
    glow::UniqueImage img (size);
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
    is(book.view.pages.size(), usize(2), "Spread has two pages");
    is(book.view.pages[1].offset.x, 7, "Spread second page has correct offset");
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
