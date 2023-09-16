#pragma once

#include "../dirt/control/command.h"
#include "common.h"

namespace app::command {
using namespace control;

///// APP COMMANDS

 // () Quit app
extern Command quit;

///// BOOK COMMANDS

 // () Go to next page(s)
extern Command next;
 // () Go to previous page(s)
extern Command prev;
 // (int32) Skip forward or backward this many pages.  The page offset will be
 // clamped to the valid range.
extern Command seek;

 // () Print the filename of the current page.  If more than one page is being
 // viewed, only prints the filename of the lowest-numbered one.
extern Command print_current_filename;

///// LAYOUT COMMANDS

 // (int32) Set the number of pages to view simultaneously
extern Command spread_pages;

 // (AutoZoomMode) Set auto zoom mode for current book
extern Command auto_zoom_mode;

 // (Vec Vec) Set alignment (small_align and large_align).  If a component of a
 // Vec is NAN, that component of the existing *_align will not be changed (so
 // you can change only the horizontal or vertical align if you want).
extern Command align;

 // (float) Multiply zoom by amount
extern Command zoom_multiply;

 // () Reset layout parameters to default (anything changed by the commands in
 // the LAYOUT COMMANDS section).
extern Command reset_layout;

///// PAGE COMMANDS

 // (InterpolationMode) Set interpolation mode for current book
extern Command interpolation_mode;

///// WINDOW COMMANDS

 // () Enter or leave fullscreen mode
extern Command fullscreen;
 // () Leave fullscreen mode or quit if not fullscreen
extern Command leave_fullscreen_or_quit;
 // (Fill) change window background fill
extern Command window_background;

} // namespace app::command
