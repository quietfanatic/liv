#include "memory.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/uni/hash.h"
#include "../dirt/uni/io.h"
#include "../dirt/uni/text.h"
#include "../dirt/uni/time.h"
#include "book.h"

namespace liv {

static IRI memory_store_location (const IRI& location) {
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
    mem.spread_range = {
        book->state.page_offset,
        book->state.page_offset +
            book->state.settings->get(&LayoutSettings::spread_count)
    };
     // TODO: read source, not block
    if (auto page = book->block.get(book->state.page_offset)) {
        mem.page = page->location.relative_to(mem.location);
    }
    else mem.page = "";
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
    try {
        ayu::load(res);
    }
    catch (std::exception& e) {
        uni::warn_utf8(cat(
            "Error loading memory file ", ayu::resource_filename(res),
            ": ", e.what(), "\n",
            "Memory of this book will be ignored or overwritten.\n"
        ));
        return;
    }
    const MemoryOfBook* mem = res.ref();
    if (mem->location != memloc) {
         // Different location with the same hash?
        ayu::force_unload(res);
        return;
    }

    if (mem->page) {
        auto start_loc = IRI(mem->page, memloc);
        for (usize i = 0; i < book->block.pages.size(); i++) {
            if (book->block.pages[i]->location == start_loc) {
                book->state.page_offset = i;
                break;
            }
        }
    }
    else {
        book->state.page_offset = mem->spread_range.l;
    }
    book->state.settings->layout.spread_count = {size(mem->spread_range)};
     // Don't need to keep this around
    ayu::force_unload(res);
}

} using namespace liv;

AYU_DESCRIBE(liv::MemoryOfBook,
    attrs(
        attr("location", &MemoryOfBook::location),
        attr("updated_at", &MemoryOfBook::updated_at),
        attr("spread_range", &MemoryOfBook::spread_range),
        attr("page", &MemoryOfBook::page),
        attr("layout", constant(null), invisible|ignore),
        attr("render", constant(null), invisible|ignore)
    )
)

#ifndef TAP_DISABLE_TESTS
#include "../dirt/tap/tap.h"

tap::TestSet tests ("liv/memory", []{
    using namespace tap;
    using namespace liv;

    App app;
    app.hidden = true;

    auto make_book = [&]{
        auto settings = std::make_unique<Settings>();
        settings->window.size = {{120, 120}};
        settings->parent = app_settings();
        auto src = std::make_unique<BookSource>(
            BookType::Folder,
            Slice<IRI>{IRI("res/liv/test/", iri::program_location())}
        );
        return std::make_unique<Book>(
            &app, move(src), move(settings)
        );
    };

    auto book = make_book();
    is(book->state.page_offset, 0);
    book->next();
    is(book->state.page_offset, 1);
    memorize_book(&*book);

    book = make_book();
    is(book->state.page_offset, 0);
    remember_book(&*book);
    is(book->state.page_offset, 1);

    done_testing();
});

#endif
