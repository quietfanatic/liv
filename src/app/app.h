#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include "../base/geo/vec.h"
#include "../base/uni/string.h"
#include "../base/wind/passive_loop.h"
#include "common.h"
#include "book.h"
#include "settings.h"

namespace app {

struct App {
    App();
    ~App();

    void open_files (std::vector<String>&& files);
    void open_list (Str filename);

    void close_book (Book*);

    void run ();
    void stop ();

     // Loaded from an ayu::Resource
    Settings* settings;

    std::vector<std::unique_ptr<Book>> books;
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
