#pragma once

#include "../base/control/command.h"

namespace app::command {
using namespace control;

 // () Go to next page(s)
extern Command next;
 // () Go to previous page(s)
extern Command prev;

 // (FitMode) Set fit mode for current book
extern Command fit_mode;

 // (InterpolationMode) Set interpolation mode for current book
extern Command interpolation_mode;

 // (float) Multiply zoom by amount
extern Command zoom_multiply;

 // () Enter or leave fullscreen mode
extern Command fullscreen;
 // () Leave fullscreen mode or quit if not fullscreen
extern Command leave_fullscreen_or_quit;

 // () Quit app
extern Command quit;

} // namespace app::command
