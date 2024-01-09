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

 // Defines and identifies a book.
struct BookSource {
    BookType type;
    UniqueArray<IRI> locations;

    void validate () const;
    BookSource () = default;
    BookSource (BookType t, UniqueArray<IRI> l) :
        type(t), locations(move(l))
    { validate(); }

     // Empty if this book should not be remembered.
     // TODO: use const IRI*
    const IRI& location_for_memory () const;
     // Current book or cwd if Misc
    const IRI& base_for_page_rel_book () const;
     // Directory containing current book or cwd if Misc
    IRI base_for_page_rel_book_parent () const;
    friend bool operator== (const BookSource& a, const BookSource& b) = default;
};

} // liv
