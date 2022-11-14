#pragma once

#include <exception>
#include "../location.h"

namespace ayu::in {

Location make_error_location (std::exception_ptr&& e);

}
