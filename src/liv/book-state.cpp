#include "book-state.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/uni/time.h"
#include "book.h"
#include "memory.h"
#include "settings.h"

namespace liv {

BookState::BookState (std::unique_ptr<Settings> s) :
    settings(move(s))
{ }

IRange BookState::viewing_range () const {
    auto spread_count = settings->get(&LayoutSettings::spread_count);
    return IRange{page_offset, page_offset + spread_count};
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

void BookState::reset_layout () {
    auto sc = settings->layout.spread_count;
    settings->layout = {};
    settings->layout.spread_count = sc;
    manual_zoom = GNAN;
    manual_offset = GNAN;
}

} using namespace liv;

AYU_DESCRIBE(liv::BookState,
    attrs(
        attr("settings", &BookState::settings, collapse_optional),
        attr("page_offset", &BookState::page_offset),
        attr("manual_zoom", &BookState::manual_zoom),
        attr("manual_offset", &BookState::manual_offset)
    )
)

