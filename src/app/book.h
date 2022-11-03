// Implements a collection of images

#pragma once

#include <memory>
#include <vector>
#include "../base/uni/common.h"
#include "settings.h"

struct SDL_Window;

namespace app {
struct App;
struct Page;

struct Book {
    App& app;

    ///// Book contents
    String folder; // empty if not in a folder
    std::vector<std::unique_ptr<Page>> pages;
    isize current_page_no = 1; // 1-based index

     // Loads all image files in the folder as pages
    explicit Book (App& app, Str folder);
     // Just loads the given files as pages
    explicit Book (App& app, const std::vector<String>& filenames);
    ~Book ();

     // Turns an invalid page number into a valid one
    isize clamp_page_no (isize no);
     // Returns null if not valid page number
    Page* get_page (isize no);

    ///// Layout decision logic
    AutoZoomMode auto_zoom_mode = FIT;
     // Controls alignment of the image when it's smaller than the window.
     // (0, 0) means the image's top-left corner is in the top-left corner of
     // the window.  (1, 1) means the image's bottom-right corner is in the
     // bottom-right corner of the window.  (0.5, 0.5) means the image's center
     // is in the center of the window.
    geo::Vec small_align;
     // Controls alignment of the image when it's larger than the window.
    geo::Vec large_align;
     // Controls texture filtering
    InterpolationMode interpolation_mode = CUBIC;

     // Clamps zoom level according to max_zoom and min_page_size
    float clamp_zoom (float);

    ///// Actual layout logic
     // Zoom has been manually changed, so ignore auto_zoom
    bool manual_zoom = false;
     // Offset has been manually changed, so ignore auto_zoom and aligns.
    bool manual_align = false;
     // Current zoom level.
    float zoom = 1;
     // In pixels, top-left origin
     // (0, 0) means image's top-left corner is in window's top-left corner
    geo::Vec offset;

    ///// Controls
     // Clamps to valid page numbers
    void set_page (isize no);
     // Increment current page by 1
     // TODO: Increment by two if viewing 2 pages (NYI)
    void next () { set_page(current_page_no + 1); }
    void prev () { set_page(current_page_no - 1); }
     // Add to current page (stopping at first/last page)
    void seek (isize count) { set_page(current_page_no + count); }

    void set_auto_zoom_mode (AutoZoomMode);
    void set_align (geo::Vec small, geo::Vec large);
    void set_interpolation_mode (InterpolationMode);
     // Adds amount to view.offset
    void drag (geo::Vec amount);

    void zoom_multiply (float factor);

    bool is_fullscreen ();
    void set_fullscreen (bool);


    ///// Internal stuff
    SDL_Window* sdl_window = null;
    void* gl_context = null;
    bool need_draw = true;
    int64 estimated_page_memory = 0;
     // Handles layout logic.  Returns true if drawing was actually done.
    bool draw_if_needed ();
     // Preload images perhaps
     // Returns true if any processing was actually done
    bool idle_processing ();

    geo::IVec get_window_size ();

    void window_size_changed (geo::IVec new_size);
};

} // namespace app
