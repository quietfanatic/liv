#include "../internal/common-internal.h"

#include <cstdlib>
#include <iostream>

#include "../../uni/utf.h"
#include "../describe.h"
#include "../serialize.h"

namespace ayu {
using namespace in;

void dump_refs (const std::vector<Reference>& rs) {
    switch (rs.size()) {
        case 0: warn_utf8("[]\n"sv); break;
        case 1: warn_utf8(item_to_string(rs[0])); break;
        default: {
            std::string r = "["s;
            r += item_to_string(rs[0]);
            for (usize i = 1; i < rs.size(); i++) {
                r += ' ';
                r += item_to_string(rs[i]);
            }
            warn_utf8(r += "]\n"sv);
            break;
        }
    }
}

} using namespace ayu;

AYU_DESCRIBE(ayu::Error,
    delegate(const_ref_func<std::source_location>(
        [](const ayu::Error& e) -> const std::source_location& {
            return *e.source_location;
        }
    ))
)

AYU_DESCRIBE(ayu::GenericError,
    elems(
        elem(base<Error>(), inherit),
        elem(&GenericError::mess)
    )
)
AYU_DESCRIBE(ayu::IOError,
    elems(
        elem(base<Error>(), inherit),
        elem(&IOError::filename),
        elem(&IOError::errnum)
    )
)
AYU_DESCRIBE(ayu::OpenFailed,
    delegate(base<Error>())
)
AYU_DESCRIBE(ayu::ReadFailed,
    delegate(base<Error>())
)
AYU_DESCRIBE(ayu::CloseFailed,
    delegate(base<Error>())
)
