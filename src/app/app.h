#pragma once

#include <memory>
#include <unordered_map>
#include "../dirt/geo/vec.h"
#include "../dirt/uni/strings.h"
#include "../dirt/wind/passive_loop.h"
#include "common.h"
#include "book.h"
#include "settings.h"

namespace app {

struct App {
    App();
    ~App();

    void open_files (AnyArray<AnyString> files);
    void open_list (AnyString filename);

    void close_book (Book*);

    void run ();
    void stop ();

     // Loaded from an ayu::Resource
    Settings* settings;

    UniqueArray<std::unique_ptr<Book>> books;
    std::unordered_map<uint32, Book*> books_by_window_id;

     // The main loop.  Need to store this here to call stop() on it.
    wind::PassiveLoop loop;

     // For testing
    bool hidden = false;
};

 // Temporal state for commands
extern App* current_app;
extern Book* current_book;

} // namespace app
