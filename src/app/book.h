// Implements a collection of images

#pragma once

#include <memory>
#include <vector>
#include "../base/wind/window.h"
#include "../base/uni/common.h"

namespace app {

struct Page;

struct Book {
    String folder; // empty if not a folder
    std::vector<std::unique_ptr<Page>> pages;
    isize current_page = 0;

     // Loads all image files in the folder as pages
    explicit Book (Str folder);
     // Just loads the given files as pages
    explicit Book (const std::vector<String>& filenames);
    ~Book ();

    wind::Window window;

     // Handles layout logic.
    void draw ();
};

} // namespace app
