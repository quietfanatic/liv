// This module implements transforming strings to trees.

#pragma once

#include "tree.h"

namespace ayu {

 // The filename parameter is used for error reporting.
 // If the parse fails, an X<ParseError> exception will be thrown.
Tree tree_from_string (Str, AnyString filename = "");

UniqueString string_from_file (AnyString filename);

Tree tree_from_file (AnyString filename);

struct ParseError : Error {
    AnyString mess;
    AnyString filename;
    uint line;
    uint col;
};

} // namespace ayu
