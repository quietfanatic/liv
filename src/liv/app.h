#pragma once

#include <memory>
#include <unordered_map>
#include "../dirt/ayu/resources/resource.h"
#include "../dirt/geo/vec.h"
#include "../dirt/uni/strings.h"
#include "../dirt/wind/passive_loop.h"
#include "common.h"
#include "settings.h"

namespace liv {

struct App {
    App();
    ~App();

     // Select between open_files, open_file, and open_folder.
    void open_args (Slice<AnyString> args, SortMethod sort = SortMethod{});
     // Open all files and folders (recursively) in a temprary book.
    void open_files (Slice<AnyString> filenames, SortMethod sort = SortMethod{});
     // Open one file as the current page, including all other files in the same
     // folder (non-recursively) as pages in a temporary book.
    void open_file (const AnyString& filename, SortMethod sort = SortMethod{});
     // Open all files in the folder (recursively) as a book.
    void open_folder (const AnyString& filename, SortMethod sort = SortMethod{});
     // Open all files and folders (recursively) written in the list
     // one-per-line as a book (temporary if filename is - for stdin).
     // This changes the CWD to the folder containing the filename (if it isn't
     // stdin).
    void open_list (const AnyString& filename, SortMethod sort = SortMethod{});

    void close_book (Book*);

    void run ();
    void stop ();

     // Loaded from ayu resources
     // TODO: make ayu::TypedResource
    ayu::Resource settings_res;
    Settings* settings;

    UniqueArray<std::unique_ptr<Book>> books;
    std::unordered_map<uint32, Book*> books_by_window_id;

     // The main loop.  Need to store this here to call stop() on it.
    wind::PassiveLoop loop;

     // To work around a bug where gaining focus from another window closing due
     // to a keystroke makes our window receive the keystroke that closed the
     // other window.
    uint32 last_focused = 0;

     // For testing
    bool hidden = false;
     // The above workaround interferes with automated input for testing, so set
     // this to true to disable the workaround.
    bool automated_input = false;
};

 // Temporal state for commands
extern App* current_app;
extern Book* current_book;

} // namespace liv
