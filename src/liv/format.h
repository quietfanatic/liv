#pragma once

#include "common.h"
#include "../dirt/uni/arrays.h"

namespace liv {

enum class FormatCommand {
    None,
    Literal,
    VisibleRange,
    PageCount,
    BookIri,
    BookAbs,
    BookRelCwd,
    BookEstMem,
    PageIri,
    PageAbs,
    PageRelCwd,
    PageRelBook,
    PageRelBookParent,
    PageFileSize,
    PagePixelWidth,
    PagePixelHeight,
    PagePixelBits,
    PageEstMem,
    MergedPagesAbs,
    MergedPagesRelCwd,
    MergedPagesRelBook,
    MergedPagesRelBookParent,
    ForVisiblePages,
    ZoomPercent,
    IfZoomed,
    Cwd,
    AppSettingsAbs,
};

struct FormatList {
    UniqueArray<FormatToken> tokens;
    FormatList (FormatList&&) = default;
    FormatList& operator= (FormatList&&) = default;
    template <class... Args>
    constexpr FormatList (Args&&... args) : tokens(
        UniqueArray<FormatToken>::make(std::forward<Args>(args)...)
    ) { }
    void write (UniqueString&, Book*) const;
    void write (UniqueString&, Book*, i32 page) const;
};

struct FormatToken {
    FormatCommand command;
    union {
        AnyString literal;
        FormatList sublist;
    };

    constexpr FormatToken () : command(FormatCommand::None) { }
    FormatToken (FormatToken&& o) {
        std::memcpy((void*)this, &o, sizeof(FormatToken));
        std::memset((void*)&o, 0, sizeof(FormatToken));
    }
    FormatToken (const AnyString& lit) :
        command(FormatCommand::Literal),
        literal(lit)
    { }
    FormatToken& operator= (FormatToken&& o) {
        this->~FormatToken();
        return *new (this) FormatToken(move(o));
    }

    FormatToken (FormatCommand cmd, FormatList&& sub = {}) :
        command(cmd)
    {
        switch (command) {
            case FormatCommand::IfZoomed:
            case FormatCommand::ForVisiblePages:
                new (&sublist) FormatList(move(sub));
                break;
            default: require(!sub.tokens);
        }
    }

    constexpr ~FormatToken () {
        switch (command) {
            case FormatCommand::Literal: literal.~AnyString(); break;
            case FormatCommand::IfZoomed:
            case FormatCommand::ForVisiblePages:
                sublist.~FormatList();
                break;
            default: break;
        }
    }

    void write (UniqueString&, Book*, i32 page) const;
};

} // liv
