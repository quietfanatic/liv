 // This stores the book state should be saved between invocations of the
 // program.

#pragma once
#include <optional>
#include "common.h"

namespace liv {

 // Returns null if this book is not remembered
std::optional<BookState> load_memory (const BookSource&);
void save_memory (const BookSource&, BookState&);

} // namespace liv
