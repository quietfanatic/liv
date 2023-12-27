#pragma once

#include "common.h"
#include "../dirt/uni/arrays.h"
#include "../dirt/iri/iri.h"

namespace liv {

enum class BookType {
    Misc,
    Folder,
    List,
    FileWithNeighbors,
};

 // Defines a book and its contents.
struct BookSource {
     // Saved
    BookType type;
     // Base IRI to resolve page filenames against.
    IRI location;
     // Not saved
    UniqueArray<IRI> pages;

     // pages will be populated with:
     //   Folder: The recursive contents of the folder.
     //   List: The recursive contents of the list.
     //   FileWithNeighbors: The non-recursive contents of the folder containing
     //     the given filename.
     //   Misc: Not allowed
    BookSource (const Settings*, BookType, const IRI&);

     // Type must be Misc.  Folders and lists will be recursively expanded.
    BookSource (const Settings*, BookType, Slice<IRI>);
     // Empty if this book should not be remembered.
    const IRI& location_for_memory ();
};

} // liv
