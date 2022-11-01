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

    String folder; // empty if not a folder
    std::vector<std::unique_ptr<Page>> pages;
    isize current_page_no = 1; // 1-based index

    FitMode fit_mode = FIT;  // Reset on page turn
    float zoom = 1;
    geo::Vec offset;  // Pixels, bottom-left origin

    wind::Window window;
    bool need_draw = true;

     // Loads all image files in the folder as pages
    explicit Book (App& app, Str folder);
     // Just loads the given files as pages
    explicit Book (App& app, const std::vector<String>& filenames);
    ~Book ();


     // Handles layout logic.
    void draw_if_needed ();
     // Change current_page
    void next ();
    void prev ();

    void set_fit_mode (FitMode);
     // Adds amount to view.offset
    void drag (geo::Vec amount);

    void zoom_multiply (float factor);

    bool is_fullscreen ();
    void set_fullscreen (bool);

    void window_size_changed (geo::IVec new_size);

     // Returns true if no is in 1..pages.size()
    bool valid_page_no (isize no);
};

} // namespace app
