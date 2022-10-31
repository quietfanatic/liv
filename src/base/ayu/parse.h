// This module implements transforming strings to trees.

#pragma once

#include "tree.h"

namespace ayu {

 // The filename parameter is used for error reporting.
Tree tree_from_string (Str, Str filename = "");

String string_from_file (Str filename);

Tree tree_from_file (Str filename);

namespace X {
    struct ParseError : Error {
        String mess;
        String filename;
        uint line;
        uint col;
        ParseError (String&& mess = "", Str f = "", uint l = 0, uint c = 0) :
            mess(std::move(mess)), filename(f), line(l), col(c)
        { }
    };
}

} // namespace ayu
