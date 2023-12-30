#pragma once

#include "common.h"
#include "../dirt/uni/arrays.h"

namespace liv {

enum class FormatCommand {
    None,
    Literal,
    VisibleRange,
    PageCount,
    PageAbs,
    ZoomPercent,
};

struct FormatToken {
    FormatCommand command;
    union {
        AnyString literal;
    };

    constexpr FormatToken () : command(FormatCommand::None) { }
    FormatToken (FormatToken&& o) {
        std::memcpy((void*)this, &o, sizeof(FormatToken));
        std::memset((void*)&o, 0, sizeof(FormatToken));
    }

    constexpr ~FormatToken () {
        switch (command) {
            case FormatCommand::Literal: literal.~AnyString(); break;
            default: break;
        }
    }

    void write (UniqueString&, Book*) const;
};

struct FormatList {
    UniqueArray<FormatToken> tokens;
    void write (UniqueString&, Book*) const;
};

} // liv
