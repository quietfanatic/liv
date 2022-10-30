#pragma once

#include "command.h"

namespace control::command {

 // (String) Print string to stdout
extern Command echo;

 // (std::vector<Statement>) Run multiple commands in a row
extern Command seq;

} // namespace control::command
