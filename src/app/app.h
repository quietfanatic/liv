#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include "../base/geo/vec.h"
#include "book.h"

namespace app {
struct Settings;

struct App {
    App();
    ~App();

    void open_folder (Str folder);
    void open_files (const std::vector<String>& files);

    void run ();
    void stop ();

     // Loaded from an ayu::Resource
    Settings* settings;

    std::vector<std::unique_ptr<Book>> books;
    std::unordered_map<uint32, Book*> books_by_window_id;
    bool stop_requested = false;

     // Temporary
    geo::IVec default_window_size = geo::IVec{640, 480};

     // For testing
    bool hidden = false;
};

 // Temporal state for commands
extern App* current_app;
extern Book* current_book;

} // namespace app
