LIV Application Commands
========================

The mappings section of the settings file includes entries mapping keypresses to
commands.  Here is the list of available commands.  See the mappings section of
res/liv/settings-default.ayu for examples of how to use these.

Generic Commands
----------------
- `[seq [<commands>]]` = Do multiple commands in sequence.

Application Commands
--------------------
- `[quit]` = Quit the application.
- `[fullscreen]` = Toggle fullscreen mode.
- `[leave_fullscreen]` = Leave fullscreen mode if currently fullscreen.
- `[leave_fullscreen_or_quit]` = If fullscreen, leave fullscreen mode, otherwise
    quit.
- `[prompt_command]` = Show a dialog box asking for a command to be entered,
    then run that command.  The command format is like it is here, but without
    the surrounding `[` and `]`.  This requires that the program `zenity` is
    installed and available in $PATH.
- `[say <FormatList>]` = Print some information to stdout from a format list,
    followed by a newline.  See formats.md for documentation on format lists.
- `[message_box <FormatList> <FormatList>]` = Show a message box with the title
    from the first format list and the content from the second.  Uses either
    zenity or SDL's builtin message box.
- `[clipboard_text <FormatList>]` = Copy formatted text to the OS clipboard.
- `[shell <FormatList>]` = Run a system shell command with a string generated
    from the given format list.  Don't forget to include spaces.
- `[run [<FormatList>...]]` = Run a system command with a separate command name
    and argument strings created from format lists.  There will be a lot of
    square brackets.  Example: `[run [[gvim] [[app_settings_abs]]]]`

Book and Page Commands
----------------------
- `[next]` = Go to the next page or pages, depending on current spread count.
- `[prev]` = Go to the previous page or pages, depending on current spread
    count.
- `[seek <int32>]` = Add to or subtract from current page number, ignoring
    current spread count.  The resulting page number will be clamped such that
    there is always at least one visible page in the spread.
- `[go_next <Direction>]` = If the direction matches the current spread
    direction, go forward by one spread count.  If it's the opposite, go
    backwards.  If perpendicular, does nothing.
- `[go <Direction> <int32>]` = If the direction matches or is opposite the
    current spread direction, seek by that much in that direction.
- `[spread_count <int32>]` = Change how many pages to view simultaneously.  The
    current maximum is 16.
- `[sort <SortMethod>]` = Change how the pages are sorted in the current book,
    keeping the current page filename the same (possibly changing the current
    page number).  See res/liv/settings-default.ayu for documentation on sort
    methods.
- `[add_to_list <String> <SortMethod>]` = Add given page to a list file at the
    given path (a file containing filenames, one per line), and then sort the
    file with the given sort method.  Duplicates will be removed unless the sort
    method is `[unsorted]`.  See res/liv/settings-default.ayu for documentation
    on sort methods.  If multiple pages are being viewed, this and below
    commands only affect the lowest-numbered page being viewed.
- `[remove_from_list <String>]` = Remove page from list file at the given path.
- `[remove_from_book]` = Remove current page from the current book.  This only
    affects the set of pages currently being tracked by the application; it does
    nothing to the page's actual file on disk.
- `[move_to_folder <String>]` = Move current page's file to the folder at the
    given path.  This does not remove the page from the current book.  To do
    both, use the `seq` command as follows:
```
[seq [
    [move_to_folder myfolder]
    [remove_from_book]
]]
```

Layout Commands
---------------
- `[spread_count <int32>]` = Set the number of pages to simultaneously view.
- `[spread_direction <Direction>]` = Set the direction to view simultaneous
    pages in.  Also affects the `[go]` and `[go_next]` commands.
- `[auto_zoom_mode <AutoZoomMode>]` = Set the auto zoom mode for the current
    book.  See res/liv/settings-default.ayu for documentation on auto zoom
    modes.
- `[align <Vec2> <Vec2>]` = Set the small and large page alignment values,
    respectively.  See `small_align` and `large_align` in
    res/liv/settings-default.ayu for more information.  If one of the components
    of the vectors is +nan, then that component will not be changed.  This is so
    you can change the horizontal alignment without touching the vertical
    alignment or vice versa.
- `[zoom_multiply <float>]` = Multiply current zoom level by the given amount.
    The zoom level will be clamped according to the `max_zoom` and
    `min_zoomed_size` settings.
- `[reset_layout]` = Reset all layout parameters that have been altered by
    commands to their default (specified in the settings files).
- `[reset_settings]` = Reset all temporary settings that have been altered by
    commands.

Render Commands
---------------
- `[interpolation_mode <InterpolationMode>]` = Set interpolation mode for
    current book.  See res/liv/settings-default.ayu for documentation on
    interpolation modes.
- `[window_background <Fill>]` = Set window background to a color.  See
    res/liv/settings-default.ayu for more information.
- `[transparency_background <Fill>]` = Set the background shown behind
    transparent images.
- `[color_range [[<float> <float>] [<float> <float>] [<float> <float>]] = Set
    the output color range.  As an example, use [[0 0.5] [0 0.5] [0 0.5]] to
    reduce the brightness to half.

