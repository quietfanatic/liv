 // This stores the book state should be saved between invocations of the
 // program.

#pragma once
#include <memory>
#include "../dirt/iri/iri.h"
#include "common.h"

namespace liv {

constexpr IRI marks_folder = IRI("data:/marks/");

 // Returns null if this book is not remembered.  If returns non-null, the
 // passed-in Settings will be moved from.
std::unique_ptr<Book> load_mark (const BookSource&, Settings&);
 // Not const Book& because we need to borrow some stuff.  We'll give it back.
void save_mark (const App&, Book&);

} // namespace liv
