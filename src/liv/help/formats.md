Format Lists
============

The window title and certain commands accept a FormatList.  This is an array
containing strings and special tokens surrounded in `[` and `]`.  Strings will
be printed exactly and the special tokens mean the following:
  `[book_abs]` = Path of book in absolute form.
  `[book_iri]` = Path of book in IRI format (file:/...)
  `[book_rel_cwd]` = Path of book relative to current working directory.
  `[book_est_mem]` = Estimated video memory for all cached pages.
  `[visible_range]` =
      Currently visible page numbers starting at 1, formatted like "1", "1,2",
      or "1-3".
  `[page_count]` = Total number of pages in the current book.
  `[page_abs]` = Path of (lowest-numbered) current page in absolute form.
  `[page_iri]` = Path of current page in IRI format (file:/...)
  `[page_rel_cwd]` = Path of current page relative to current working directory.
  `[page_rel_book]` = Path of current page relative to book path.
  `[page_rel_book_parent]` =
      Path of current page relative to the folder containing the book.  This
      only differs from `page_rel_book` if the book is itself a folder.
  `[page_file_size]` = Filesize of current page on disk.
  `[page_pixel_width]` = Width of current image in pixels.
  `[page_pixel_height]` = Height of current image in pixels.
  `[page_pixel_bits]` = Bits-per-pixel of current image (e.g. 24 for RGB8).
  `[page_est_mem]` =
      Estimated video memory usage of current page; Width * height * bits/8.
  `[page_load_time]` = Time in seconds it took to load the page
  `[merged_pages_abs]` =
      All page paths in absolute form merged together like
      /home/foo/bar{01,02}.png
  `[merged_pages_rel_cwd]` = Merged page paths relative to CWD.
  `[merged_pages_rel_book]` = Merged page paths relative to book path.
  `[merged_pages_rel_book_parent]` = See `[page_rel_book_parent]`.
  `[for_visible_pages <more>...]` =
      Show <more> for each visible page instead of just the first.
  `[zoom_percent]` = Current zoom level times 100.
  `[if_zoomed <more>...]` = Show <more> if zoom level is not 1.0 (100%).
  `[cwd]` = Current working directory.
  `[app_settings_abs]` = Path to global application settings file.
