#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include "book.h"

namespace app {

struct App {
    void open_folder (Str folder);
    void open_files (const std::vector<String>& files);

    void run ();
    void stop ();

    std::vector<std::unique_ptr<Book>> books;
    std::unordered_map<uint32, Book*> books_by_window_id;
    bool stop_requested = false;
};

} // namespace app
