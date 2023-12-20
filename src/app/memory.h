#include "common.h"
#include "layout.h"
#include "page.h"
#include "../dirt/uni/arrays.h"

namespace app {

struct MemoryOfBook {
    AnyString book_filename;  // filename of either folder or list
    AnyString current_filename;  // filename of current page (relative to book_filename)
    LayoutParams layout_params;
    PageParams page_params;
    double last_viewed_at = 0;
    int32 current_offset = 0;
     // Non-semantic
    bool need_write = false;
};

struct Memory {
    UniqueArray<MemoryOfBook> books;
};

} // namespace app
