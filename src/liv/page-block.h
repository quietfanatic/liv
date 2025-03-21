#pragma once

#include <vector>
#include <memory>
#include "../dirt/geo/range.h"
#include "../dirt/uni/common.h"
#include "common.h"

namespace liv {

// Implements a collection of Pages.  Responsible for gathering filenames from
// folders, keeping track of which pages are loaded, estimating memory usage of
// loaded pages.
struct PageBlock {
    UniqueArray<std::unique_ptr<Page>> pages;
    i64 estimated_page_memory = 0;

    PageBlock () = default;
    PageBlock (PageBlock&&) = default;
    PageBlock (const PageBlock&) = delete;
    PageBlock (const BookSource&, const Settings&);
    ~PageBlock ();

    void resort (SortMethod);

     // Returns null if i is out of range
    Page* get (i32 i) const;
     // Returns -1 if there's no page with this location
    i32 find (const IRI&) const;
    i32 count () const { return i32(size(pages)); }

    IRange valid_pages () const { return {0, count()}; }

    void load_page (Page*);
    void unload_page (Page*);

     // Preload pages perhaps
     // Returns true if any processing was actually done.
    bool idle_processing (const Book*, const Settings&);
};

} // namespace liv
