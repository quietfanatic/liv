#pragma once

#include "common.h"
#include "../dirt/control/command-base.h"
#include "../dirt/control/statement.h"

namespace liv {

struct Command : control::CommandBase<Command, void, Book&> {
    using control::CommandBase<Command, void, Book&>::CommandBase;
};

struct Statement : control::Statement<Command> {
    using control::Statement<Command>::Statement;
};

} // liv
