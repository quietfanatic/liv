 // This stores the book state should be saved between invocations of the
 // program.

#pragma once

#include "common.h"
#include "layout.h"
#include "page.h"
#include "../dirt/iri/iri.h"
#include "../dirt/uni/arrays.h"
#include "../dirt/geo/range.h"

namespace liv {

struct MemoryOfBook {
    IRI location;  // location of either folder or list
    AnyString current_page;  // location of current page (relative to book_filename)
    LayoutParams layout_params;
    PageParams page_params;
    double updated_at = 0;
    IRange current_range;
};

struct Memory {
    UniqueArray<MemoryOfBook> books;
     // Non-semantic
    bool need_write = false;

    void remember_book (const Book*);
};

} // namespace liv
