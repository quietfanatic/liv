#include "format.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/traversal/from-tree.h"
#include "../dirt/ayu/traversal/to-tree.h"
#include "../dirt/uni/hash.h"
#include "book.h"

//TEMP
#include "../dirt/uni/io.h"

namespace liv {

 // Merge multiple paths together with a format like foo/bar-{01,02}.png
static
void merge_paths (UniqueString& s, Slice<UniqueString> paths) {
    if (!paths) return;
    if (paths.size() == 1) {
         // Only one path given so just print it.
        encat(s, paths[0]);
        return;
    }
     // Find longest common prefix and suffix
    usize prefix = paths[0].size();
    usize suffix = paths[0].size();
    for (auto& path : paths.slice(1)) {
        for (usize i = 0; i < prefix && i < path.size(); i++) {
            if (path[i] != paths[0][i]) {
                prefix = i;
                break;
            }
        }
        for (usize i = 0; i < suffix && i < path.size(); i++) {
            if (path[path.size() - i - 1] !=
                paths[0][paths[0].size() - i - 1]
            ) {
                suffix = i;
                break;
            }
        }
    }
     // Oh but don't chop up numbers
    while (prefix > 0 &&
        paths[0][prefix - 1] >= '0' && paths[0][prefix - 1] <= '9'
    ) prefix--;
    while (suffix > 0 &&
        paths[0][paths[0].size() - suffix] >= '0' &&
        paths[0][paths[0].size() - suffix] <= '9'
    ) suffix--;
    if (prefix + suffix > paths[0].size()) {
         // If the prefix and suffix overlap, we must have been given identical
         // paths.  I don't know what to do with this situation!  For now we'll
         // do something safish.
        suffix = 0;
    }
     // Now do it
    encat(s,
        paths[0].slice(0, prefix), '{',
        Caterator(",", paths.size(), [&paths, prefix, suffix](usize i){
            return paths[i].slice(prefix, paths[i].size() - suffix);
        }),
        '}', paths[0].slice(paths[0].size() - suffix)
    );
}

NOINLINE
void FormatToken::write (UniqueString& s, Book* book, int32 page) const {
    switch (command) {
        case FormatCommand::None: break;
        case FormatCommand::Literal:
            encat(s, literal);
            break;
        case FormatCommand::VisibleRange: {
            auto visible = book->visible_range();
            if (size(visible) == 0) {
                 // Dunno what to do here
                encat(s, 0);
            }
            else if (size(visible) == 1) {
                encat(s, visible.l+1);
            }
            else {
                char sep = size(visible) == 2 ? ',' : '-';
                encat(s, visible.l+1, sep, visible.r+1 - 1);
            }
            break;
        }
        case FormatCommand::PageCount:
            encat(s, book->block.count());
            break;
        case FormatCommand::BookAbs: {
            auto&& loc = book->source->location_for_memory();
            if (loc) {
                encat(s, iri::to_fs_path(loc));
            }
            break;
        }
        case FormatCommand::BookRelCwd: {
            auto&& loc = book->source->location_for_memory();
            if (loc) {
                auto&& rel = loc.relative_to(iri::working_directory());
                encat(s, iri::decode_path(rel));
            }
            break;
        }
        case FormatCommand::BookEstMem: {
            encat(s, (book->block.estimated_page_memory + 1023) / 1024, 'K');
            break;
        }
        case FormatCommand::PageAbs: {
            if (page < 0) break;
            auto&& loc = book->block.pages[page]->location;
            encat(s, iri::to_fs_path(loc));
            break;
        }
        case FormatCommand::PageRelCwd: {
            if (page < 0) break;
            auto&& loc = book->block.pages[page]->location;
            auto&& rel = loc.relative_to(iri::working_directory());
            encat(s, iri::decode_path(rel));
            break;
        }
        case FormatCommand::PageRelBook: {
            if (page < 0) break;
            auto&& loc = book->block.pages[page]->location;
            auto&& base = book->source->base_for_page_rel_book();
            auto&& rel = loc.relative_to(base);
            encat(s, iri::decode_path(rel));
            break;
        }
        case FormatCommand::PageRelBookParent: {
            if (page < 0) break;
            auto&& loc = book->block.pages[page]->location;
            auto&& base = book->source->base_for_page_rel_book_parent();
            auto&& rel = loc.relative_to(base);
            encat(s, iri::decode_path(rel));
            break;
        }
        case FormatCommand::PageFileSize: {
            if (page < 0) break;
            std::error_code code;
            auto&& loc = book->block.pages[page]->location;
            auto size = fs::file_size(iri::to_fs_path(loc), code);
            if (size == decltype(size)(-1)) {
                encat(s, "(unavailable)");
            }
            else {
                encat(s, (size + 1023) / 1024, "K");
            }
            break;
        }
        case FormatCommand::PagePixelWidth: {
            if (page < 0) break;
            auto size = book->block.get(page)->size;
            encat(s, size.x);
            break;
        }
        case FormatCommand::PagePixelHeight: {
            if (page < 0) break;
            auto size = book->block.get(page)->size;
            encat(s, size.y);
            break;
        }
        case FormatCommand::PagePixelBits: {
            if (page < 0) break;
            auto depth = book->block.get(page)->texture->bpp();
            encat(s, depth);
            break;
        }
        case FormatCommand::PageEstMem: {
            if (page < 0) break;
            encat(s, (book->block.get(page)->estimated_memory + 1023) / 1024, 'K');
            break;
        }
        case FormatCommand::MergedPagesAbs: {
            auto visible = book->visible_range();
            if (!size(visible)) break;
            auto paths = UniqueArray<UniqueString>(size(visible), [=](usize i){
                auto&& loc = book->block.pages[visible[i]]->location;
                return iri::to_fs_path(loc);
            });
            merge_paths(s, paths);
            break;
        }
        case FormatCommand::MergedPagesRelCwd: {
            auto visible = book->visible_range();
            if (!size(visible)) break;
            auto paths = UniqueArray<UniqueString>(size(visible), [=](usize i){
                auto&& loc = book->block.pages[visible[i]]->location;
                auto&& rel = loc.relative_to(iri::working_directory());
                return iri::decode_path(rel);
            });
            merge_paths(s, paths);
            break;
        }
        case FormatCommand::MergedPagesRelBook: {
            auto visible = book->visible_range();
            if (!size(visible)) break;
            auto&& base = book->source->base_for_page_rel_book();
            auto paths = UniqueArray<UniqueString>(
                size(visible), [=, &base](usize i)
            {
                auto&& loc = book->block.pages[visible[i]]->location;
                auto&& rel = loc.relative_to(base);
                return iri::decode_path(rel);
            });
            merge_paths(s, paths);
            break;
        }
        case FormatCommand::MergedPagesRelBookParent: {
            auto visible = book->visible_range();
            if (!size(visible)) break;
            auto&& base = book->source->base_for_page_rel_book_parent();
            auto paths = UniqueArray<UniqueString>(
                size(visible), [=, &base](usize i)
            {
                auto&& loc = book->block.pages[visible[i]]->location;
                auto&& rel = loc.relative_to(base);
                return iri::decode_path(rel);
            });
            merge_paths(s, paths);
            break;
        }
        case FormatCommand::ForVisiblePages: {
            for (auto p : book->visible_range()) {
                sublist.write(s, book, p);
            }
            break;
        }
        case FormatCommand::Cwd: {
            encat(s, iri::to_fs_path(iri::working_directory()));
            break;
        }
        case FormatCommand::ZoomPercent: {
            float zoom = book->view.get_layout().zoom;
            encat(s, round(zoom * 100));
            break;
        }
        case FormatCommand::IfZoomed: {
            if (book->view.get_layout().zoom != 1) {
                sublist.write(s, book, page);
            }
            break;
        }
        default: never();
    }
}


void FormatList::write (UniqueString& s, Book* book) const {
    auto visible = book->visible_range();
    write(s, book, size(visible) ? visible.l : -1);
}
NOINLINE
void FormatList::write (UniqueString& r, Book* book, int32 page) const {
    for (auto& token : tokens) token.write(r, book, page);
}

static ayu::Tree FormatToken_to_tree (const FormatToken& v){
    using namespace ayu;
    switch (v.command) {
        case FormatCommand::None: return Tree::array();
        case FormatCommand::Literal: return Tree(v.literal);
        case FormatCommand::IfZoomed: {
            auto a = AnyArray<Tree>(item_to_tree(&v.sublist));
            a.insert(usize(0), Tree("if_zoomed"));
            return Tree(move(a));
        }
        case FormatCommand::ForVisiblePages: {
            auto a = AnyArray<Tree>(item_to_tree(&v.sublist));
            a.insert(usize(0), Tree("for_visible_pages"));
            return Tree(move(a));
        }
        default: {
            return Tree(UniqueArray<Tree>::make(item_to_tree(&v.command)));
        }
    }
}

static void FormatToken_from_tree (FormatToken& v, const ayu::Tree& t) {
    using namespace ayu;
    v = {};
    if (t.form == Form::String) {
        v.command = FormatCommand::Literal;
        new (&v.literal) AnyString(t);
    }
    else if (t.form == Form::Array) {
        auto a = Slice<Tree>(t);
        if (!a) {
            v.command = FormatCommand::None;
            return;
        }
        item_from_tree(&v.command, a[0]);
        switch (v.command) {
            case FormatCommand::IfZoomed:
            case FormatCommand::ForVisiblePages: {
                new (&v.sublist) FormatList();
                auto args = AnyArray(a.slice(1));
                item_from_tree(
                    &v.sublist, ayu::Tree(move(args)),
                    ayu::Location(), ayu::DELAY_SWIZZLE
                );
                break;
            }
            default: {
                if (a.size() != 1) {
                    raise_LengthRejected(
                        ayu::Type::CppType<FormatToken>(),
                        1, 1, a.size()
                    );
                }
                break;
            }
        }
    }
}

} using namespace liv;

AYU_DESCRIBE(liv::FormatCommand,
    values(
         // Leaving out None and Literal
        value("visible_range", FormatCommand::VisibleRange),
        value("page_count", FormatCommand::PageCount),
        value("book_abs", FormatCommand::BookAbs),
        value("book_rel_cwd", FormatCommand::BookRelCwd),
        value("book_est_mem", FormatCommand::BookEstMem),
        value("page_abs", FormatCommand::PageAbs),
        value("page_rel_cwd", FormatCommand::PageRelCwd),
        value("page_rel_book", FormatCommand::PageRelBook),
        value("page_rel_book_parent", FormatCommand::PageRelBookParent),
        value("page_file_size", FormatCommand::PageFileSize),
        value("page_pixel_width", FormatCommand::PagePixelWidth),
        value("page_pixel_height", FormatCommand::PagePixelHeight),
        value("page_pixel_bits", FormatCommand::PagePixelBits),
        value("page_est_mem", FormatCommand::PageEstMem),
        value("merged_pages_abs", FormatCommand::MergedPagesAbs),
        value("merged_pages_rel_cwd", FormatCommand::MergedPagesRelCwd),
        value("merged_pages_rel_book", FormatCommand::MergedPagesRelBook),
        value("merged_pages_rel_book_parent", FormatCommand::MergedPagesRelBookParent),
        value("for_visible_pages", FormatCommand::ForVisiblePages),
        value("cwd", FormatCommand::Cwd),
        value("zoom_percent", FormatCommand::ZoomPercent),
        value("if_zoomed", FormatCommand::IfZoomed)
    )
)

AYU_DESCRIBE(liv::FormatToken,
    to_tree(&FormatToken_to_tree),
    from_tree(&FormatToken_from_tree)
)

AYU_DESCRIBE(liv::FormatList,
    delegate(member(&FormatList::tokens))
)

#ifndef TAP_DISABLE_TESTS
#include "../dirt/tap/tap.h"

static tap::TestSet tests ("liv/format", []{
    using namespace tap;

    fs::current_path(iri::to_fs_path(iri::program_location().chop_filename()));

    Str fmt_ayu =
        "["
            "[merged_pages_rel_cwd] "
            "\" [\" [visible_range] / [page_count] \"]\" "
            "[if_zoomed \" (\" [zoom_percent] \"%)\"]"
        "]";
    FormatList fmt;
    ayu::item_from_string(&fmt, fmt_ayu);
    is(ayu::item_to_string(&fmt), fmt_ayu, "FormatList AYU round-trip");

    UniqueString s;
    merge_paths(s, Slice<UniqueString>{
        "foobarbaz0123.jpeg", "foobarbaz0124.jpeg", "foobarbaz0125.jpeg"
    });
    is(s, "foobarbaz{0123,0124,0125}.jpeg", "merge_paths");

    auto settings = std::make_unique<Settings>();
    settings->window.size = {120, 120};
    settings->window.hidden = true;
    settings->parent = app_settings();
    App app;
    auto src = std::make_unique<BookSource>(
        BookType::Misc, Slice<IRI>{
            IRI("res/liv/test/image.png", iri::program_location()),
            IRI("res/liv/test/image2.png", iri::program_location())
        }
    );
    Book book (&app, move(src), BookState(move(settings)));

    UniqueString got;
    fmt.write(got, &book);
    StaticString expected = "res/liv/test/image.png [1/2] (1714%)";
    is(got, expected, "FormatList::write 1");

    book.next();
    book.auto_zoom_mode(AutoZoomMode::Original);
    got = "";
    fmt.write(got, &book);
    expected = "res/liv/test/image2.png [2/2]";
    is(got, expected, "FormatList::write 2");

    book.prev();
    book.spread_count(2);
    got = "";
    fmt.write(got, &book);
    expected = "res/liv/test/image{,2}.png [1,2/2]";
    is(got, expected, "FormatList::write 3");

    done_testing();
});

#endif
