// Implements a collection of Pages.  State includes whether those Pages are
// loaded or not (actually, the Pages themselves include that), and the total
// estimated video memory of all pages.  Does not include the current page or
// any view parameters.  Pages are indexed by 1.

#pragma once

#include <vector>
#include <memory>
#include "../base/uni/common.h"
#include "common.h"

namespace app {

struct PageRange {
    isize offset;
    isize spread_pages;
};

struct Pages {
    String folder; // empty if not in a folder
    std::vector<std::unique_ptr<Page>> pages;
    int64 estimated_page_memory = 0;

    Pages (FilesToOpen&);
    ~Pages ();

     // Turns an invalid page offset into a valid one
    isize clamp_page_offset (PageRange) const;
    isize first_visible_page (PageRange) const;
    isize last_visible_page (PageRange) const;
     // Returns null if no is not in 1..pages.size()
    isize count () const { return isize(pages.size()); }
    Page* get (isize no) const;
    void load_page (Page*);
    void unload_page (Page*);
     // Preload pages perhaps
     // Returns true if any processing was actually done
    bool idle_processing (const Settings* settings, PageRange);
};

} // namespace app
