// Implements a collection of Pages.  State includes whether those Pages are
// loaded or not (actually, the Pages themselves include that), and the total
// estimated video memory of all pages.  Does not include the current page or
// any view parameters.  Pages are indexed by 0.

#pragma once

#include <vector>
#include <memory>
#include "../dirt/geo/range.h"
#include "../dirt/uni/common.h"
#include "common.h"

namespace liv {

struct PageBlock {
    UniqueArray<std::unique_ptr<Page>> pages;
    int64 estimated_page_memory = 0;

    PageBlock (const BookSource&);
    ~PageBlock ();

     // Returns null if i is out of range
    Page* get (int32 i) const;
    int32 count () const { return int32(size(pages)); }

    IRange valid_pages () const { return {0, count()}; }

    void load_page (Page*);
    void unload_page (Page*);
     // Preload pages perhaps
     // Returns true if any processing was actually done.
    bool idle_processing (const Book* book, const Settings* settings);
};

} // namespace liv
