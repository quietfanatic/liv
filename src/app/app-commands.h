#pragma once

#include "../base/control/command.h"

namespace app::command {
using namespace control;

 // () Go to previous page(s)
extern Command prev;
 // () Go to next page(s)
extern Command next;
 // () Quit app
extern Command quit;

 // (FitMode) Set fit mode for current book
extern Command fit_mode;

 // () Enter or leave fullscreen mode
extern Command fullscreen;
 // () Leave fullscreen mode or quit if not fullscreen
extern Command leave_fullscreen_or_quit;

} // namespace app::command
