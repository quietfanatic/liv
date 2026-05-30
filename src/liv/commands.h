#pragma once

#include "common.h"
#include "../dirt/cmd/command-base.h"
#include "../dirt/cmd/instruction.h"

namespace liv {

struct Command : cmd::CommandBase<Command, void(Book&)> {
    using cmd::CommandBase<Command, void(Book&)>::CommandBase;
};

struct Instruction : cmd::Instruction<Command> {
    using cmd::Instruction<Command>::Instruction;
};

} // liv
