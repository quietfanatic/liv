// This module implements transforming trees into strings.

#pragma once

#include "common.h"
#include "tree.h"

namespace ayu {

using PrintOptions = uint32;
enum : PrintOptions {
     // Print with a compact layout.  This is the default for tree_to_string.
    COMPACT = 1 << 0,
     // Print with a pretty layout.  This is the default for tree_to_file.
    PRETTY = 1 << 1,
     // Print in JSON-compatible format.  This option is NOT WELL TESTED so it
     // may produce non-conforming output.
    JSON = 1 << 2,
     // For validation
    VALID_PRINT_OPTION_BITS = COMPACT | PRETTY | JSON
};

String tree_to_string (const Tree&, PrintOptions opts = 0);

void string_to_file (Str, Str filename);

void tree_to_file (const Tree&, Str filename, PrintOptions opts = 0);

 // Conflicting combination of print options was provided, or it had bits
 // outside of VALID_PRINT_OPTION_BITS.
struct InvalidPrintOptions : Error {
    PrintOptions opts;
};

} // namespace ayu
