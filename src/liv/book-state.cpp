#include "book-state.h"

#include "../dirt/uni/time.h"
#include "book.h"
#include "memory.h"
#include "settings.h"

namespace liv {

BookState::BookState (Book* b, const Memory* memory) :
    book(b),
    window_background(
        book->app->settings->get(&WindowSettings::window_background)
    )
{
    auto settings = book->app->settings;
    auto source = &*book->source;
     // Figure out where to start and initialize view params from memory if
     // applicable.
    const MemoryOfBook* mem = null;
    IRI start;
    if (auto& memloc = source->location_for_memory()) {
        for (auto& m : memory->books) {
            if (m.location == memloc) {
                mem = &m;
                break;
            }
        }
    }
    else if (source->type == BookType::FileWithNeighbors) {
        start = source->location;
    }
    if (mem) {
        layout_params = mem->layout_params;
        page_params = mem->page_params;
        start = IRI(mem->current_page, mem->location);
    }
    else {
        layout_params = LayoutParams(settings);
        page_params = PageParams(settings);
    }
    int32 start_index = 0;
    if (start) {
        for (usize i = 0; i < source->pages.size(); i++) {
            if (source->pages[i] == start) {
                start_index = i;
                break;
            }
        }
    }
    else if (mem) {
        start_index = mem->current_range.l;
    }
    auto spread_count = mem ? size(mem->current_range) :
        settings->get(&LayoutSettings::spread_count);
    spread_range = {start_index, start_index + spread_count};
}

MemoryOfBook BookState::make_memory () const {
    MemoryOfBook mem;
    mem.location = book->source->location_for_memory();
    mem.current_range = spread_range;
    if (auto page = book->block.get(spread_range.l)) {
        mem.current_page = page->location.relative_to(mem.location);
    }
    else mem.current_page = "";
    mem.layout_params = layout_params;
    mem.page_params = page_params;
    mem.updated_at = uni::now();
    return mem;
}

IRange BookState::visible_range () const {
    return spread_range & IRange{0, book->source->pages.size()};
}

void BookState::set_page_number (int32 no) {
    auto settings = book->app->settings;
    auto source = &*book->source;
     // Clamp such that there is at least one visible page in the range
    int32 l = clamp(
        no - 1,
        1 - int32(size(spread_range)),
        int32(source->pages.size()) - 1
    );
    spread_range = {l, l + size(spread_range)};
    expect(size(visible_range()) >= 1);
    if (settings->get(&LayoutSettings::reset_zoom_on_page_turn)) {
        layout_params.manual_zoom = GNAN;
        layout_params.manual_offset = GNAN;
    }
}

void BookState::set_spread_count (int32 count) {
     // TODO: clamp spread_range.l too
    spread_range.r = spread_range.l + clamp(count, 1, 2048);
}

void BookState::set_auto_zoom_mode (AutoZoomMode mode) {
    layout_params.auto_zoom_mode = mode;
    layout_params.manual_zoom = GNAN;
    layout_params.manual_offset = GNAN;
}

void BookState::set_align (geo::Vec small, geo::Vec large) {
    if (defined(small.x)) layout_params.small_align.x = small.x;
    if (defined(small.y)) layout_params.small_align.y = small.y;
    if (defined(large.x)) layout_params.large_align.x = large.x;
    if (defined(large.y)) layout_params.large_align.y = large.y;
    layout_params.manual_offset = GNAN;
}

void BookState::drag (geo::Vec amount) {
    if (!defined(layout_params.manual_offset)) {
         // TODO: do get_layout() in Book
        auto& layout = book->view.get_layout();
        layout_params.manual_offset = layout.offset;
        layout_params.manual_zoom = layout.zoom;
    }
    layout_params.manual_offset += amount;
}

void BookState::zoom_multiply (float factor) {
     // TODO: do the decision-making that depends on BookView in Book instead of
     // here, so that state doesn't depend on view.
     // Need spread to clamp the zoom
    auto& spread = book->view.get_spread();
     // Actually we also need the layout to multiply the zoom
    auto& layout = book->view.get_layout();
     // Set manual zoom
    layout_params.manual_zoom = spread.clamp_zoom(
        book->app->settings, layout.zoom * factor
    );
    if (defined(layout_params.manual_offset)) {
         // Hacky way to zoom from center
         // TODO: zoom to preserve current alignment
        layout_params.manual_offset +=
            spread.size * (layout.zoom - layout_params.manual_zoom) / 2;
    }
}

void BookState::reset_layout () {
    layout_params = LayoutParams(book->app->settings);
}

} // namespace liv
