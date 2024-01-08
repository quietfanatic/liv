 // This stores the book state should be saved between invocations of the
 // program.

#pragma once

#include "common.h"
#include "layout.h"
#include "page.h"
#include "../dirt/iri/iri.h"
#include "../dirt/geo/range.h"

namespace liv {

struct MemoryOfBook {
    IRI location;  // location of either folder or list
    AnyString page;  // location of current page (relative to book_filename)
    LayoutParams layout;
    double updated_at = 0;  // For eviction (which is NYI)
    IRange spread_range;
};

void memorize_book (const Book*);
void remember_book (Book*);

} // namespace liv
