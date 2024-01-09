#include "memory.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/traversal/to-tree.h"
#include "../dirt/uni/hash.h"
#include "../dirt/uni/io.h"
#include "../dirt/uni/text.h"
#include "../dirt/uni/time.h"
#include "book.h"

namespace liv {

struct MemoryOfBook {
    BookSource source;
    BookState state;
    double updated_at = 0;
};

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

static void ensure_memory_folder () {
    fs::create_directory(
        ayu::resource_filename("data:/memory")
    );
}

std::optional<BookState> load_memory (const BookSource& src) {
    auto& loc = src.location_for_memory();
    if (!loc) return {};

    ensure_memory_folder();

    auto res = ayu::Resource(memory_store_location(loc));
    if (!ayu::source_exists(res)) return {};
    try {
        ayu::load(res);
    }
    catch (std::exception& e) {
        uni::warn_utf8(cat(
            "Error loading memory file ", ayu::resource_filename(res),
            ": ", e.what(), "\n",
            "Memory of this book will be ignored or overwritten.\n"
        ));
        return {};
    }
    MemoryOfBook* mem = res.ref();
    if (mem->source != src) [[unlikely]] {
        uni::warn_utf8(cat(
            "Hash collision in memory file ", ayu::resource_filename(res),
            ".\nOld source: ", ayu::item_to_string(&mem->source),
            "\nNew source: ", ayu::item_to_string(&src),
            "\nOld memory will be overwritten with new memory.\n"
        ));
         // Hash collision?
        ayu::force_unload(res);
        return {};
    }
    std::optional<BookState> r {move(mem->state)};
     // Don't need to keep this around
    ayu::force_unload(res);
    return r;
}

void save_memory (const BookSource& source, BookState& state) {
    auto& loc = source.location_for_memory();
    if (!loc) return;

    ensure_memory_folder();

    MemoryOfBook mem {source, move(state), now()};
    mem.updated_at = uni::now();

     // Doing it with the following line causes breakage because the impicit
     // coercion of move(mem) to ayu::Dynamic moves from mem before mem.location
     // is accessed.
    //auto res = ayu::Resource(memory_store_location(mem.source.locations[0]), move(mem));
    auto store = memory_store_location(mem.source.locations[0]);
    auto res = ayu::Resource(store, move(mem));
    try {
        ayu::save(res);
    }
    catch (std::exception& e) {
        uni::warn_utf8(cat(
            "Failed to save memory file ", ayu::resource_filename(res),
            ": ", e.what(), "\nMemory of this book will not be saved.\n"
        ));
    }
    MemoryOfBook* saved = res.ref();
     // Give the state back
    state = move(saved->state);
    ayu::force_unload(res);
}

} using namespace liv;

AYU_DESCRIBE(liv::MemoryOfBook,
    attrs(
        attr("source", &MemoryOfBook::source),
        attr("state", &MemoryOfBook::state),
        attr("updated_at", &MemoryOfBook::updated_at)
    )
)

#ifndef TAP_DISABLE_TESTS
#include "../dirt/tap/tap.h"

tap::TestSet tests ("liv/memory", []{
    using namespace tap;
    using namespace liv;

    App app;
    app.hidden = true;

    auto settings = std::make_unique<Settings>();
    settings->window.size = {{120, 120}};
    settings->parent = app_settings();
    auto src = std::make_unique<BookSource>(
        BookType::Folder,
        Slice<IRI>{IRI("res/liv/test/", iri::program_location())}
    );
    BookState to_save (move(settings));
    to_save.page_offset = 1;
    to_save.settings->layout.auto_zoom_mode = AutoZoomMode::FitWidth;

    doesnt_throw([&]{
        save_memory(*src, to_save);
    }, "save_memory");

    BookState to_load;
    doesnt_throw([&]{
        to_load = *load_memory(*src);
    }, "load_memory");
    is(to_load.page_offset, 1);
    is(to_load.settings->layout.auto_zoom_mode, AutoZoomMode::FitWidth);

    done_testing();
});

#endif
