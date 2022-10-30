#pragma once

#include "../base/control/command.h"
#include "../base/control/input.h"
#include "view.h"

namespace app {

struct Mapping {
    control::Input input;
    control::Statement action;
};

struct Settings {
    View default_view;
    std::vector<Mapping> mappings;
};

} // namespace app
