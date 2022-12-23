 // This module contains cross-system compatibility functions, mostly UTF-8
 // related functions for use on Windows.  There are apparently ways to get
 // Windows programs to use UTF-8 encoding for IO, but I can't get it to work.

#pragma once

#include <cstdio>

#include "common.h"
#include "string.h"

namespace uni {

///// UTF-8/UTF-16 CONVERSION

 // UTF-8/UTF-16 conversion functions.  These are best-effort, and never throw
 // errors, instead passing invalid characters through.  Unmatched UTF-8 bytes
 // and overlong sequences are treated as Latin-1 characters, and unmatched
 // UTF-16 surrogtes are encoded as-is into UTF-8.   UTF-16 is native-endian.

 // Convert a UTF-8 string into a native-endian UTF-16 string.
std::u16string to_utf16 (Str);

 // Convert a native-endian UTF-16 string into a UTF=8 string.
std::string from_utf16 (Str16);

///// UTF-8 IO FUNCTIONS

 // fopen but UTF-8 even on Windows
std::FILE* fopen_utf8 (const char* filename, const char* mode = "rb");

 // Print UTF-8 formatted text to a file.  Might not fuse starting or trailing
 // umatched UTF-8 bytes between calls.
 // TODO: These will stop at NUL bytes, fix that.
void fprint_utf8 (std::FILE* f, Str s);
 // Prints to stdout and flushes.
void print_utf8 (Str s);
 // Prints to stderr and flushes.
void warn_utf8 (Str s);

 // Delete a file
int remove_utf8 (const char* filename);

} // namespace uni
