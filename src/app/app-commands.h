#pragma once

#include "../base/control/command.h"

namespace app::command {
using namespace control;

 // () Go to next page(s)
extern Command next;
 // () Go to previous page(s)
extern Command prev;
 // () Quit app
extern Command quit;

 // (FitMode) Set fit mode for current book
extern Command fit_mode;

 // () Enter or leave fullscreen mode
extern Command fullscreen;
 // () Leave fullscreen mode or quit if not fullscreen
extern Command leave_fullscreen_or_quit;

 // (float) Multiply zoom by amount
extern Command zoom_multiply;

} // namespace app::command