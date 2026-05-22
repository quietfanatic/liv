#pragma once

#include "common.h"
#include "../dirt/cmd/command-base.h"
#include "../dirt/cmd/statement.h"

namespace liv {

struct Command : cmd::CommandBase<Command, void(Book&)> {
    using cmd::CommandBase<Command, void(Book&)>::CommandBase;
};

struct Statement : cmd::Statement<Command> {
    using cmd::Statement<Command>::Statement;
};

} // liv
