// This module implements transforming strings to trees.

#pragma once

#include "tree.h"

namespace ayu {

 // The filename parameter is used for error reporting.
Tree tree_from_string (Str, Str filename = ""sv);

String string_from_file (Str filename);

Tree tree_from_file (Str filename);

namespace X {
    struct ParseError : LogicError {
        String mess;
        String filename;
        uint line;
        uint col;
        ParseError (String&& mess = ""s, Str f = ""sv, uint l = 0, uint c = 0) :
            mess(std::move(mess)), filename(f), line(l), col(c)
        { }
    };
}

} // namespace ayu
