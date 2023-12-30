#pragma once

#include "../dirt/uni/arrays.h"
#include "../dirt/iri/iri.h"
#include "common.h"
#include "settings.h"

namespace liv {

enum class BookType {
    Misc,
    Folder,
    List,
    FileWithNeighbors,
};

 // Defines a book and its contents.
struct BookSource {
    BookType type;
    IRI location;
    UniqueArray<IRI> pages;

     // pages will be populated with:
     //   Folder: The recursive contents of the folder.
     //   List: The recursive contents of the list.
     //   FileWithNeighbors: The non-recursive contents of the folder containing
     //     the given filename.
     //   Misc: Not allowed
    BookSource (const Settings*, BookType, const IRI&, SortMethod sort = SortMethod{});

     // Type must be Misc.  Folders and lists will be recursively expanded.
    BookSource (const Settings*, BookType, Slice<IRI>, SortMethod sort = SortMethod{});
     // Empty if this book should not be remembered.
    const IRI& location_for_memory ();
};

} // liv
