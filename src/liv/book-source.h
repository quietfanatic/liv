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

    BookSource () = default;
    BookSource (BookType t, UniqueArray<IRI> l) :
        type(t), locations(move(l))
    { validate(); }

    void validate ();
     // Empty if this book should not be remembered.
    const IRI& location_for_mark () const;
     // Current book or cwd if Misc
    const IRI& base_for_page_rel_book () const;
     // Directory containing current book or cwd if Misc
    IRI base_for_page_rel_book_parent () const;
    friend bool operator== (const BookSource& a, const BookSource& b) = default;
};

} // liv
