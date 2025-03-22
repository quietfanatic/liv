#include "book-state.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/uni/time.h"
#include "book.h"
#include "settings.h"

namespace liv {

BookState::BookState (std::unique_ptr<Settings> s) :
    settings(move(s))
{ }

IRange BookState::viewing_range () const {
    auto spread_count = settings->get(&LayoutSettings::spread_count);
    return IRange{page_offset, page_offset + spread_count};
}

} using namespace liv;

AYU_DESCRIBE(liv::BookState,
    attrs(
        attr("settings", &BookState::settings, collapse_optional),
        attr("page_offset", &BookState::page_offset),
        attr("manual_zoom", &BookState::manual_zoom, collapse_optional),
        attr("manual_offset", &BookState::manual_offset, collapse_optional)
    )
)

