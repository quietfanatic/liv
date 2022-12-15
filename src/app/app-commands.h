#pragma once

#include "../base/control/command.h"
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
 // (isize) Skip forward or backward this many pages
extern Command seek;

///// PAGE COMMANDS

 // (AutoZoomMode) Set auto zoom mode for current book
extern Command auto_zoom_mode;

 // (Vec Vec) Set alignment (small_align and large_align)
extern Command align;

 // (InterpolationMode) Set interpolation mode for current book
extern Command interpolation_mode;

 // (float) Multiply zoom by amount
extern Command zoom_multiply;

 // () Reset layout parameters to default
extern Command reset_layout;

///// WINDOW COMMANDS

 // () Enter or leave fullscreen mode
extern Command fullscreen;
 // () Leave fullscreen mode or quit if not fullscreen
extern Command leave_fullscreen_or_quit;
 // (Fill) change window background fill
extern Command window_background;

} // namespace app::command
