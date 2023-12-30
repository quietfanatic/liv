#include "format.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/traversal/from-tree.h"
#include "../dirt/ayu/traversal/to-tree.h"
#include "../dirt/uni/hash.h"
#include "book.h"

namespace liv {

void FormatToken::write (UniqueString& s, Book* book) const {
    switch (command) {
        case FormatCommand::None: break;
        case FormatCommand::Literal:
            s = cat(move(s), literal);
            break;
        case FormatCommand::VisibleRange: {
            auto visible = book->state.visible_range();
            if (size(visible) == 0) {
                 // Dunno what to do here
                s = cat(move(s), 0);
            }
            else if (size(visible) == 1) {
                s = cat(move(s), visible.l+1);
            }
            else {
                char sep = size(visible) == 2 ? ',' : '-';
                s = cat(move(s), visible.l+1, sep, visible.r+1 - 1);
            }
            break;
        }
        case FormatCommand::PageCount:
            s = cat(move(s), book->block.count());
            break;
        case FormatCommand::PageAbs: {
            auto visible = book->state.visible_range();
            auto abs = iri::to_fs_path(book->block.get(visible.l)->location);
            s = cat(move(s), abs);
            break;
        }
        case FormatCommand::ZoomPercent: {
            float zoom = book->view.get_layout().zoom;
            s = cat(move(s), round(zoom * 100));
            break;
        }
        case FormatCommand::IfZoomed: {
            if (book->view.get_layout().zoom != 1) {
                sublist.write(s, book);
            }
            break;
        }
        default: never();
    }
}

void FormatList::write (UniqueString& r, Book* book) const {
    for (auto& token : tokens) token.write(r, book);
}

static ayu::Tree FormatToken_to_tree (const FormatToken& v){
    using namespace ayu;
    switch (v.command) {
        case FormatCommand::None: return Tree(TreeArray());
        case FormatCommand::Literal: return Tree(v.literal);
        case FormatCommand::VisibleRange:
            return Tree(TreeArray::make(Tree("visible_range")));
        case FormatCommand::PageCount:
            return Tree(TreeArray::make(Tree("page_count")));
        case FormatCommand::PageAbs:
            return Tree(TreeArray::make(Tree("page_abs")));
        case FormatCommand::ZoomPercent:
            return Tree(TreeArray::make(Tree("zoom_percent")));
        case FormatCommand::IfZoomed: {
            auto a = TreeArray(item_to_tree(&v.sublist));
            a.insert(usize(0), Tree("if_zoomed"));
            return Tree(move(a));
        }
        default: never();
    }
}

static void FormatToken_from_tree (FormatToken& v, const ayu::Tree& t) {
    using namespace ayu;
    expect(v.command == FormatCommand::None);
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
        auto cmd = Str(a[0]);
        switch (uni::hash32(cmd)) {
            case uni::hash32("visible_range"): {
                v.command = FormatCommand::VisibleRange;
                goto no_args;
            }
            case uni::hash32("page_count"): {
                v.command = FormatCommand::PageCount;
                goto no_args;
            }
            case uni::hash32("page_abs"): {
                v.command = FormatCommand::PageAbs;
                goto no_args;
            }
            case uni::hash32("zoom_percent"): {
                v.command = FormatCommand::ZoomPercent;
                goto no_args;
            }
            case uni::hash32("if_zoomed"): {
                v.command = FormatCommand::IfZoomed;
                goto args;
            }
            default: raise(e_General, "Invalid format command");
        }
        return;
        no_args:
        if (a.size() != 1) {
            raise_LengthRejected(ayu::Type::CppType<FormatToken>(), 1, 1, a.size());
        }
        return;
        args:
        new (&v.sublist) UniqueArray<FormatToken>();
        auto args = TreeArray(a.slice(1));
        item_from_tree(
            &v.sublist, ayu::Tree(move(args)), ayu::Location(), ayu::DELAY_SWIZZLE
        );
    }
}

} using namespace liv;

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
    book.auto_zoom_mode(AutoZoomMode::ORIGINAL);
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
