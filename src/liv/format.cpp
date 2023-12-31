#include "format.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/traversal/from-tree.h"
#include "../dirt/ayu/traversal/to-tree.h"
#include "../dirt/uni/hash.h"
#include "book.h"

namespace liv {

NOINLINE
void FormatToken::write (UniqueString& s, Book* book, int32 page) const {
    switch (command) {
        case FormatCommand::None: break;
        case FormatCommand::Literal:
            encat(s, literal);
            break;
        case FormatCommand::VisibleRange: {
            auto visible = book->state.visible_range();
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
            auto& loc = book->source->location_for_memory();
            if (loc) {
                encat(s, iri::to_fs_path(loc));
            }
            break;
        }
        case FormatCommand::BookRelCwd: {
            auto& loc = book->source->location_for_memory();
            if (loc) {
                auto rel = loc.relative_to(iri::working_directory());
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
            auto& loc = book->source->pages[page];
            encat(s, iri::to_fs_path(loc));
            break;
        }
        case FormatCommand::PageRelCwd: {
            if (page < 0) break;
            auto& loc = book->source->pages[page];
            auto rel = loc.relative_to(iri::working_directory());
            encat(s, iri::decode_path(rel));
            break;
        }
        case FormatCommand::PageRelBook: {
            if (page < 0) break;
            auto& loc = book->source->pages[page];
            auto rel = loc.relative_to(book->source->location);
            encat(s, iri::decode_path(rel));
            break;
        }
        case FormatCommand::PageFileSize: {
            if (page < 0) break;
            std::error_code code;
            auto size = fs::file_size(iri::to_fs_path(
                book->source->pages[page]
            ), code);
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
        case FormatCommand::ForVisiblePages: {
            for (auto& p : book->state.visible_range()) {
                sublist.write(s, book, p);
            }
            break;
        }
        default: never();
    }
}


void FormatList::write (UniqueString& s, Book* book) const {
    auto visible = book->state.visible_range();
    write(s, book, size(visible) ? visible.l : -1);
}
NOINLINE
void FormatList::write (UniqueString& r, Book* book, int32 page) const {
    for (auto& token : tokens) token.write(r, book, page);
}

static ayu::Tree FormatToken_to_tree (const FormatToken& v){
    using namespace ayu;
    switch (v.command) {
        case FormatCommand::None: return Tree(TreeArray());
        case FormatCommand::Literal: return Tree(v.literal);
        case FormatCommand::IfZoomed: {
            auto a = TreeArray(item_to_tree(&v.sublist));
            a.insert(usize(0), Tree("if_zoomed"));
            return Tree(move(a));
        }
        case FormatCommand::ForVisiblePages: {
            auto a = TreeArray(item_to_tree(&v.sublist));
            a.insert(usize(0), Tree("for_visible_pages"));
            return Tree(move(a));
        }
        default: {
            return Tree(TreeArray::make(item_to_tree(&v.command)));
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
        auto a = TreeArraySlice(t);
        if (!a) {
            v.command = FormatCommand::None;
            return;
        }
        item_from_tree(&v.command, a[0]);
        switch (v.command) {
            case FormatCommand::IfZoomed:
            case FormatCommand::ForVisiblePages: {
                new (&v.sublist) FormatList();
                auto args = TreeArray(a.slice(1));
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
        value("page_file_size", FormatCommand::PageFileSize),
        value("page_pixel_width", FormatCommand::PagePixelWidth),
        value("page_pixel_height", FormatCommand::PagePixelHeight),
        value("page_pixel_bits", FormatCommand::PagePixelBits),
        value("page_est_mem", FormatCommand::PageEstMem),
        value("zoom_percent", FormatCommand::ZoomPercent),
        value("if_zoomed", FormatCommand::IfZoomed),
        value("for_visible_pages", FormatCommand::ForVisiblePages)
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

    Str fmt_ayu =
        "[\"[\" [visible_range] / [page_count] \"] \" "
        "[page_abs] [if_zoomed \" (\" [zoom_percent] \"%)\"]]";
    FormatList fmt;
    ayu::item_from_string(&fmt, fmt_ayu);
    is(ayu::item_to_string(&fmt), fmt_ayu, "FormatList AYU round-trip");

    App app;
    app.hidden = true;
    app.settings->WindowSettings::size = {120, 120};
    Book book (
        &app, std::make_unique<BookSource>(
            app.settings, BookType::Misc, Slice<IRI>{
                IRI("res/liv/test/image.png", iri::program_location()),
                IRI("res/liv/test/image2.png", iri::program_location())
            }
        )
    );

    UniqueString got;
    fmt.write(got, &book);
    auto expected = cat(
        "[1/2] ", iri::to_fs_path(
            IRI("res/liv/test/image.png", iri::program_location())
        ), " (1714%)"
    );
    is(got, expected, "FormatList::write 1");

    book.next();
    book.auto_zoom_mode(AutoZoomMode::Original);
    got = "";
    fmt.write(got, &book);
    expected = cat(
        "[2/2] ", iri::to_fs_path(
            IRI("res/liv/test/image2.png", iri::program_location())
        )
    );
    is(got, expected, "FormatList::write 2");

    done_testing();
});

#endif
