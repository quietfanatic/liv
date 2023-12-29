#include "memory.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/uni/hash.h"
#include "../dirt/uni/text.h"
#include "../dirt/uni/time.h"
#include "book.h"

namespace liv {

static IRI memory_store_location (IRI location) {
    expect(location);
    uint64 hash = uni::hash64(location.spec());
    char hex [16];
    for (usize i = 0; i < 16; i++) {
        hex[i] = uni::to_hex_digit(hash >> 60);
        hash <<= 4;
    }
    return IRI(cat(
        "data:/memory/", Str(hex, 16), ".ayu"
    ));
}

void ensure_memory_folder () {
    fs::create_directory(
        ayu::resource_filename("data:/memory")
    );
}

void memorize_book (const Book* book) {
    auto& memloc = book->source->location_for_memory();
    if (!memloc) return;

    ensure_memory_folder();

    MemoryOfBook mem;
    mem.location = memloc;
    mem.current_range = book->state.spread_range;
     // TODO: read source, not block
    if (auto page = book->block.get(book->state.spread_range.l)) {
        mem.current_page = page->location.relative_to(mem.location);
    }
    else mem.current_page = "";
    mem.layout_params = book->state.layout_params;
    mem.page_params = book->state.page_params;
    mem.updated_at = uni::now();

     // Doing it with the following line causes breakage because the
     // ayu::Dynamic(move(mem)) clears mem before mem.location is accessed.
    //auto res = ayu::Resource(memory_store_location(mem.location), move(mem));
    auto store = memory_store_location(mem.location);
    auto res = ayu::Resource(store, move(mem));
    ayu::save(res);
    ayu::force_unload(res);
}

void remember_book (Book* book) {
    auto& memloc = book->source->location_for_memory();
    if (!memloc) return;

    ensure_memory_folder();

    auto res = ayu::Resource(memory_store_location(memloc));
    if (!ayu::source_exists(res)) return;
    const MemoryOfBook* mem = res.ref();
    if (mem->location != memloc) {
         // Different location with the same hash?
        return;
    }

    book->state.layout_params = mem->layout_params;
    book->state.page_params = mem->page_params;

    int32 start_index = 0;
    if (mem->current_page) {
        auto start_loc = IRI(mem->current_page, memloc);
        for (usize i = 0; i < book->source->pages.size(); i++) {
            if (book->source->pages[i] == start_loc) {
                start_index = i;
                break;
            }
        }
    }
    else {
        start_index = mem->current_range.l;
    }
    book->state.spread_range = {
        start_index, start_index + size(mem->current_range)
    };
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

#ifndef TAP_DISABLE_TESTS
#include "../dirt/tap/tap.h"

tap::TestSet tests ("liv/memory", []{
    using namespace tap;
    using namespace liv;

    App app;
    app.hidden = true;
    app.settings->WindowSettings::size = {120, 120};

    auto make_book = [&]{
        return std::make_unique<Book>(
            &app, std::make_unique<BookSource>(
                app.settings, BookType::Folder,
                IRI("res/liv/test/", iri::program_location())
            )
        );
    };

    auto book = make_book();
    is(book->state.spread_range.l, 0);
    book->next();
    is(book->state.spread_range.l, 1);
    memorize_book(&*book);

    book = make_book();
    is(book->state.spread_range.l, 0);
    remember_book(&*book);
    is(book->state.spread_range.l, 1);

    done_testing();
});

#endif
