#include "memory.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/uni/time.h"
#include "book.h"

namespace liv {

void Memory::remember_book (const Book* book) {
    if (auto& memloc = book->source->location_for_memory()) {
        auto mem = book->state.make_memory();
        need_write = true;
        for (auto& m : books) {
            if (m.location == memloc) {
                m = move(mem);
                return;
            }
        }
        books.emplace_back(move(mem));
    }
}

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
