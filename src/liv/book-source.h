#pragma once

#include "common.h"
#include "../dirt/uni/arrays.h"

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
    AnyString name;
     // Not saved
     // Currently these are relative to the current working directory.
     // TODO: Make them relative to the book's folder.
    UniqueArray<AnyString> pages;

     // pages will be populated with:
     //   Folder: The recursive contents of the folder.
     //   List: The recursive contents of the list.
     //   FileWithNeighbors: The non-recursive contents of the folder containing
     //     the given filename.
     //   Misc: Not allowed
    BookSource (const Settings*, BookType, const AnyString&);

     // Type must be Misc.  Folders and lists will be recursively expanded.
    BookSource (const Settings*, BookType, Slice<AnyString>);
     // Empty if this book should not be remembered.
    const AnyString& name_for_memory ();
};

} // liv
