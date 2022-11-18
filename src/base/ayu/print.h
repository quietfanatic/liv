// This module implements transforming trees into strings.

#pragma once

#include "common.h"
#include "tree.h"

namespace ayu {

enum PrintOptionsEnum : uint32 {
     // Print with a compact layout.  This is the default for tree_to_string.
    COMPACT = 1 << 0,
     // Print with a pretty layout.  This is the default for tree_to_file.
    PRETTY = 1 << 1,
    VALID_PRINT_OPTION_BITS = COMPACT | PRETTY
};
using PrintOptions = uint32;

String tree_to_string (const Tree&, PrintOptions opts = 0);

void string_to_file (Str, Str filename);

void tree_to_file (const Tree&, Str filename, PrintOptions opts = 0);

namespace X {
     // Invalid combination of print options was provided.
    struct InvalidPrintOptions : Error {
        PrintOptions opts;
        InvalidPrintOptions (PrintOptions opts) : opts(opts) { }
    };
}

} // namespace ayu
