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

} // namespace app::command
