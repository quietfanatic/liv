// Implements a collection of images

#pragma once

#include <memory>
#include <vector>
#include "../base/wind/window.h"
#include "../base/uni/common.h"
#include "settings.h"

namespace app {
struct App;
struct Page;

struct Book {
    App& app;

    View view;

    String folder; // empty if not a folder
    std::vector<std::unique_ptr<Page>> pages;
    isize current_page_no = 1; // 1-based index

     // Loads all image files in the folder as pages
    explicit Book (App& app, Str folder);
     // Just loads the given files as pages
    explicit Book (App& app, const std::vector<String>& filenames);
    ~Book ();

    wind::Window window;

     // Handles layout logic.
    void draw ();
     // Change current_page
    void next ();
    void prev ();

     // Adds amount to offset
    void drag (geo::Vec amount);

    void zoom_multiply (float factor);

     // Returns true if no is in 1..pages.size()
    bool valid_page_no (isize no);
};

} // namespace app
