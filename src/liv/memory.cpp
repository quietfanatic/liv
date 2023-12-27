#include "memory.h"
#include "../dirt/ayu/reflection/describe.h"

namespace liv {

} using namespace liv;

AYU_DESCRIBE(liv::MemoryOfBook,
    attrs(
        attr("location", &MemoryOfBook::location),
        attr("updated_at", &MemoryOfBook::updated_at),
        attr("current_range", &MemoryOfBook::current_range),
        attr("current_page", &MemoryOfBook::current_page),
        attr("layout_params", &MemoryOfBook::layout_params),
        attr("page_params", &MemoryOfBook::page_params)
    )
)

AYU_DESCRIBE(liv::Memory,
    attrs(
        attr("books", &Memory::books)
    )
)
