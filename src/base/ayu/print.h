// This module implements transforming trees into strings.

#pragma once

#include "tree.h"

namespace ayu {

enum PrintFlags {
    COMPACT = 1,
};

 // The filename parameter is used for error reporting.
String tree_to_string (const Tree&, PrintFlags flags = PrintFlags(0));

void string_to_file (Str, Str filename);

void tree_to_file (const Tree&, Str filename, PrintFlags flags = PrintFlags(0));

} // namespace ayu
