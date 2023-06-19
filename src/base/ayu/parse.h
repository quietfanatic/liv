// This module implements transforming strings to trees.

#pragma once

#include "tree.h"

namespace ayu {

 // The filename parameter is used for error reporting.
 // If the parse fails, an X<ParseError> exception will be thrown.  TODO: Make a
 // non-throwing function.
Tree tree_from_string (OldStr, OldStr filename = ""sv);

std::string string_from_file (OldStr filename);

Tree tree_from_file (OldStr filename);

struct ParseError : Error {
    std::string mess;
    std::string filename;
    uint line;
    uint col;
};

} // namespace ayu
