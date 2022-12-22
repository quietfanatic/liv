#include "common.h"

#include <iostream>

namespace uni {
using namespace std::literals;

[[gnu::cold]]
void throw_requirement_failed (std::source_location loc) {
    throw RequirementFailed(loc);
}
[[gnu::cold]]
void abort_requirement_failed (std::source_location loc) {
    std::cerr << RequirementFailed(loc).what() << std::endl;
    std::abort();
}

[[gnu::cold]]
const char* RequirementFailed::what () const noexcept {
    mess_cache = "ERROR: require() failed at "s + loc.file_name()
               + ':' + std::to_string(loc.line())
               + "\n       in " + loc.function_name();
    return mess_cache.c_str();
}

} using namespace uni;
