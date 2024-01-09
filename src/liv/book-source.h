#pragma once

#include "../dirt/uni/arrays.h"
#include "../dirt/iri/iri.h"
#include "common.h"

namespace liv {

enum class BookType {
    Misc, // TODO: rename to Args
    Folder,
    List,
    FileWithNeighbors,
};

 // Defines and identifies what a book is.
struct BookSource {
    BookType type;
    UniqueArray<IRI> locations;

    void validate ();
    BookSource (BookType t, UniqueArray<IRI> l) :
        type(t), locations(move(l))
    { validate(); }

     // Empty if this book should not be remembered.
     // TODO: use const IRI*
    const IRI& location_for_memory ();
     // Current book or cwd if Misc
    const IRI& base_for_page_rel_book ();
     // Directory containing current book or cwd if Misc
    IRI base_for_page_rel_book_parent ();
};

} // liv
