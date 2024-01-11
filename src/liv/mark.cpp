#include "mark.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/traversal/scan.h"
#include "../dirt/ayu/traversal/to-tree.h"
#include "../dirt/uni/hash.h"
#include "../dirt/uni/io.h"
#include "../dirt/uni/text.h"
#include "../dirt/uni/time.h"
#include "book.h"

namespace liv {

struct Mark {
    BookSource source;
    BookState state;
    IRI page;
    double saved_at = 0;
};

static IRI get_mark_location (const IRI& location) {
    expect(location);
     // Make sure save folder exists
    fs::create_directory(
        ayu::resource_filename(marks_folder)
    );
    uint64 hash = uni::hash64(location.spec());
    char hex [16];
    for (usize i = 0; i < 16; i++) {
        hex[i] = uni::to_hex_digit(hash >> 60);
        hash <<= 4;
    }
    return IRI(cat(Str(hex, 16), ".ayu"), marks_folder);
}

std::unique_ptr<Book> load_mark (const BookSource& src, Settings& settings) {
    auto& loc = src.location_for_mark();
    if (!loc) return null;

     // Load resource from disk
    auto res = ayu::Resource(get_mark_location(loc));
    if (!ayu::source_exists(res)) return null;
    try {
        ayu::load(res);
    }
    catch (std::exception& e) {
        uni::warn_utf8(cat(
            "Error loading mark file ", ayu::resource_filename(res),
            ": ", e.what(), "\n",
            "Mark file for this book will be ignored or overwritten.\n"
        ));
        return null;
    }
    Mark* mark = res.ref();
     // Check for hash collision
    if (mark->source != src) [[unlikely]] {
        uni::warn_utf8(cat(
            "Hash collision in mark file ", ayu::resource_filename(res),
            ".\nOld source: ", ayu::item_to_string(&mark->source),
            "\nNew source: ", ayu::item_to_string(&src),
            "\nOld mark will be overwritten with new mark.\n"
        ));
        ayu::force_unload(res);
        return null;
    }
     // Apply command-line setting overrides
    mark->state.settings->merge(move(settings));
     // Find start page
    PageBlock block (mark->source, *mark->state.settings);
    int32 index = block.find(mark->page);
    if (index >= 0) mark->state.page_offset = index;
     // Assemble the book
    auto r = std::make_unique<Book>(
        move(mark->source),
        move(block),
        move(mark->state)
    );
     // Don't need to keep this around
    ayu::force_unload(res);
    return r;
}

void save_mark (Book& book) {
    auto& loc = book.source.location_for_mark();
    if (!loc) return;

    IRI page_loc;
    if (auto page = book.block.get(book.state.page_offset)) {
        page_loc = page->location;
    }

    auto res = ayu::Resource(
        get_mark_location(loc),
        ayu::Dynamic::make<Mark>(
            move(book.source), move(book.state), move(page_loc), now()
        )
    );

    try {
         // Most if not all memory files will have settings/parent set to the
         // app settings.
        static auto app_settings_loc =
            ayu::location_from_iri(app_settings_location);
        ayu::PushLikelyReference plr (
            app_settings(), app_settings_loc
        );
        ayu::save(res);
    }
    catch (std::exception& e) {
        uni::warn_utf8(cat(
            "Failed to save save file ", ayu::resource_filename(res),
            ": ", e.what(), "\nSave file for this book will not be saved.\n"
        ));
    }

     // Give book it's insides back
    Mark* mark = res.ref();
    book.source = move(mark->source);
    book.state = move(mark->state);
     // Don't keep resource loaded
    ayu::force_unload(res);
}

} using namespace liv;

AYU_DESCRIBE(liv::Mark,
    attrs(
        attr("source", &Mark::source),
        attr("state", &Mark::state, include),
        attr("page", mixed_funcs<AnyString>(
            [](const Mark& v){
                expect(v.source.locations.size() == 1);
                return AnyString(v.page.relative_to(v.source.locations[0]));
            },
            [](Mark& v, const AnyString& s){
                expect(v.source.locations.size() == 1);
                v.page = IRI(s, v.source.locations[0]);
            }
        )),
        attr("saved_at", &Mark::saved_at, optional)
    )
)

#ifndef TAP_DISABLE_TESTS
#include "../dirt/tap/tap.h"

tap::TestSet tests ("liv/mark", []{
    using namespace tap;
    using namespace liv;

    auto src = BookSource(
        BookType::Folder,
        Slice<IRI>{IRI("res/liv/test/", iri::program_location())}
    );

    auto settings = std::make_unique<Settings>();
    settings->window.hidden = true;
    settings->layout.auto_zoom_mode = AutoZoomMode::FitWidth;
    settings->parent = app_settings();

    Book to_save (move(src), move(settings));
    to_save.state.page_offset = 0;
    save_mark(to_save);
    pass("save_mark");

    auto overrides = std::make_unique<Settings>();
    auto sort = SortMethod{SortCriterion::Natural, SortFlags::Reverse};
    overrides->files.sort = sort;

    std::unique_ptr<Book> loaded = load_mark(to_save.source, *overrides);
    ok(!!loaded, "load_mark");
    is(loaded->source, to_save.source, "Source is same");
    is(loaded->state.page_offset, 1, "Kept page even when sort order was changed");
    is(loaded->state.settings->files.sort, sort, "Setting override is applied");
    is(loaded->state.settings->layout.auto_zoom_mode, AutoZoomMode::FitWidth,
        "Non-overridden setting is remembered"
    );

     // And save it again differently to make sure that we aren't reading the
     // mark file from the previous run.  In addition, this page offset is out
     // of range, so it should be preserved instead of overwritten with the
     // index of the page with the same location.
    to_save.state.page_offset = -1;
    save_mark(to_save);
     // Reusing overrides which has been moved from but we don't care
    loaded = load_mark(to_save.source, *overrides);
    is(loaded->state.page_offset, -1);

    done_testing();
});

#endif
