// This module implements transforming strings to trees.

#pragma once

#include "tree.h"

namespace ayu {

 // The filename parameter is used for error reporting.
Tree tree_from_string (Str, Str filename = ""sv);

String string_from_file (Str filename);

Tree tree_from_file (Str filename);

struct ParseError : Error {
    String mess;
    String filename;
    uint line;
    uint col;
};

} // namespace ayu
