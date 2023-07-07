#pragma once

#include "command.h"

namespace control::command {

 // (AnyString) Print string to stdout
extern Command echo;

 // (UniqueArray<Statement>) Run multiple commands in a row
extern Command seq;

 // (std::optional<Anystring>) Get help about all commands or a specific
 // command.  NYI
extern Command help;

} // namespace control::command
