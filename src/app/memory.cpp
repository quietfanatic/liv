#include "memory.h"
#include "../dirt/ayu/reflection/describe.h"

namespace app {

} using namespace app;

AYU_DESCRIBE(app::MemoryOfBook,
    attrs(
        attr("book_filename", &MemoryOfBook::book_filename),
        attr("updated_at", &MemoryOfBook::updated_at),
        attr("current_filename", &MemoryOfBook::current_filename),
        attr("current_offset", &MemoryOfBook::current_offset),
        attr("layout_params", &MemoryOfBook::layout_params),
        attr("page_params", &MemoryOfBook::page_params)
    )
)

AYU_DESCRIBE(app::Memory,
    attrs(
        attr("books", &Memory::books)
    )
)
