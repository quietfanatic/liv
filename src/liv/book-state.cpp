#include "book-state.h"

#include "../dirt/uni/time.h"
#include "book.h"
#include "memory.h"
#include "settings.h"

namespace liv {

BookState::BookState (Book* b, std::unique_ptr<Settings> s) :
    book(b),
    settings(move(s))
{
    if (book->source->type == BookType::FileWithNeighbors) {
        for (usize i = 0; i < book->source->pages.size(); i++) {
            if (book->source->pages[i] == book->source->location) {
                page_offset = i;
                break;
            }
        }
    }
    plog("set up state");
}

IRange BookState::visible_range () const {
    auto spread_count = settings->get(&LayoutSettings::spread_count);
    auto viewing = IRange{page_offset, page_offset + spread_count};
    auto valid = IRange{0, book->source->pages.size()};
    return viewing & valid;
}

void BookState::set_page_offset (int32 off) {
    auto source = &*book->source;
    auto spread_count = settings->get(&LayoutSettings::spread_count);
     // Clamp such that there is at least one visible page in the range
    page_offset = clamp(
        off,
        1 - int32(spread_count),
        int32(source->pages.size()) - 1
    );
    expect(size(visible_range()) >= 1);
    if (settings->get(&LayoutSettings::reset_zoom_on_page_turn)) {
        manual_zoom = GNAN;
        manual_offset = GNAN;
    }
}

void BookState::set_spread_count (int32 count) {
    settings->layout.spread_count = {clamp(count, 1, 2048)};
     // Reclamp page_offset
    set_page_offset(page_offset);
}

void BookState::set_auto_zoom_mode (AutoZoomMode mode) {
    settings->layout.auto_zoom_mode = {mode};
    manual_zoom = GNAN;
    manual_offset = GNAN;
}

void BookState::set_align (geo::Vec small, geo::Vec large) {
    auto small_align = settings->get(&LayoutSettings::small_align);
    auto large_align = settings->get(&LayoutSettings::large_align);
    if (defined(small.x)) small_align.x = small.x;
    if (defined(small.y)) small_align.y = small.y;
    if (defined(large.x)) large_align.x = large.x;
    if (defined(large.y)) large_align.y = large.y;
    settings->layout.small_align = {small_align};
    settings->layout.large_align = {large_align};
    manual_offset = GNAN;
}

void BookState::drag (geo::Vec amount) {
    if (!defined(manual_offset)) {
         // TODO: do get_layout() in Book
        auto& layout = book->view.get_layout();
        manual_offset = layout.offset;
        manual_zoom = layout.zoom;
    }
    manual_offset += amount;
}

void BookState::zoom_multiply (float factor) {
     // TODO: do the decision-making that depends on BookView in Book instead of
     // here, so that state doesn't depend on view.
     // Need spread to clamp the zoom
    auto& spread = book->view.get_spread();
     // Actually we also need the layout to multiply the zoom
    auto& layout = book->view.get_layout();
     // Set manual zoom
    manual_zoom = spread.clamp_zoom(
        *settings, layout.zoom * factor
    );
    if (defined(manual_offset)) {
         // Hacky way to zoom from center
         // TODO: zoom to preserve current alignment instead
        manual_offset +=
            spread.size * (layout.zoom - manual_zoom) / 2;
    }
}

void BookState::reset_layout () {
     // Reset everything but spread_count
    settings->layout = { .spread_count = settings->layout.spread_count };
    manual_zoom = GNAN;
    manual_offset = GNAN;
}

} // namespace liv
