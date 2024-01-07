#pragma once

#include "../dirt/control/command.h"
#include "common.h"

namespace liv::commands {
using namespace control;

///// APP AND WINDOW COMMANDS

 // () Quit app
extern Command quit;
 // () Enter or leave fullscreen mode
extern Command fullscreen;
 // () Leave fullscreen mode
extern Command leave_fullscreen;
 // () Leave fullscreen mode or quit if not fullscreen
extern Command leave_fullscreen_or_quit;
 // () Show a dialog box prompting for a command.
extern Command prompt_command;

///// BOOK AND PAGE COMMANDS

 // () Go to next page(s)
extern Command next;
 // () Go to previous page(s)
extern Command prev;
 // (int32) Skip forward or backward this many pages.  The page offset will be
 // clamped to the valid range.
extern Command seek;

 // (FormatList) Print the information in the format list to stdout, followed by
 // a newline.  See settings-default.ayu#/window/title for more info on format
 // lists.
extern Command say;

 // (FormatList, FormatList) Show a message box with the title formatted by the
 // first format list and the message by the second.
extern Command message_box;

 // (AnyString, SortMethod) Add current page to list file, sorting file by the
 // given method.
extern Command add_to_list;

 // (AnyString) Remove current page from list file.
extern Command remove_from_list;

 // (FormatList) Copy formatted info to the OS clipboard.
extern Command clipboard_text;

 // () Remove current page from current book; doesn't touch the file on disk.
extern Command remove_from_book;

 // (AnyString) Move current page file to the given folder.  Doesn't remove the
 // page from the book.  To do both together use
 //     [seq [[move_to_folder folder] [remove_from_book]]]
extern Command move_to_folder;

 // (SortMethod) Change sort order of current book, preserving which page is
 // currently being viewed.
extern Command sort;

///// LAYOUT COMMANDS

 // (int32) Set the number of pages to view simultaneously
extern Command spread_pages;

 // (AutoZoomMode) Set auto zoom mode for current book
extern Command auto_zoom_mode;

 // (Vec, Vec) Set alignment (small_align and large_align).  If a component of a
 // Vec is NAN, that component of the existing *_align will not be changed (so
 // you can change only the horizontal or vertical align if you want).
extern Command align;

 // (float) Multiply zoom by amount
extern Command zoom_multiply;

 // () Reset layout parameters to default (anything changed by the commands in
 // the LAYOUT COMMANDS section).
extern Command reset_layout;

///// RENDER COMMANDS

 // (InterpolationMode) Set interpolation mode for current book
extern Command interpolation_mode;
 // (Fill) change window background fill
extern Command window_background;

} // namespace liv::commands
