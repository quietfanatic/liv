#include "common.h"
#include "layout.h"
#include "page.h"
#include "../dirt/uni/arrays.h"
#include "../dirt/geo/range.h"

namespace liv {

struct MemoryOfBook {
    AnyString book_filename;  // filename of either folder or list
    AnyString current_filename;  // filename of current page (relative to book_filename)
    LayoutParams layout_params;
    PageParams page_params;
    double updated_at = 0;
    IRange current_range;
};

struct Memory {
    UniqueArray<MemoryOfBook> books;
     // Non-semantic
    bool need_write = false;
};

} // namespace liv
