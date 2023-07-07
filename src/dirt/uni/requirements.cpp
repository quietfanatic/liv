#include "requirements.h"

#include "strings.h"
#include "utf.h"

#include <iostream>

namespace uni {
inline namespace requirements {

[[gnu::cold]]
void abort_requirement_failed (std::source_location loc) {
    warn_utf8(cat(
        "ERROR: require() failed at ", loc.file_name(), ':',
        loc.line(), " in ", loc.function_name()
    ));
    std::abort();
}

} // requirements
} // uni
