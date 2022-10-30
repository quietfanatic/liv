#pragma once

#include "../base/control/command.h"
#include "../base/control/input.h"

namespace app {

struct Mapping {
    control::Input input;
    control::Statement action;
};

struct Settings {
    std::vector<Mapping> mappings;
};

} // namespace app
