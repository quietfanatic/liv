#include "../internal/common-internal.h"

#include <cstdlib>
#include <iostream>

#include "../compat.h"
#include "../describe.h"
#include "../serialize.h"

using namespace std::literals;

namespace ayu {
using namespace in;

void dump_refs (const std::vector<Reference>& rs) {
    switch (rs.size()) {
        case 0: warn_utf8("[]\n"); break;
        case 1: warn_utf8(item_to_string(rs[0])); break;
        default: {
            std::string r = "[";
            r += item_to_string(rs[0], COMPACT);
            for (usize i = 1; i < rs.size(); i++) {
                r += " " + item_to_string(rs[i], COMPACT);
            }
            warn_utf8(r + "]\n");
            break;
        }
    }
}

namespace X {
    const char* Error::what () const noexcept {
        if (mess_cache.empty()) {
            auto& cppt = typeid(*this);
            if (Type t = get_description_by_type_info(cppt)) {
                String s = "[" + t.name() + ' ';
                {
                    DiagnosticSerialization ds;
                     // TODO: this is INCORRECT and will BREAK without downcast
                    s += item_to_string(Reference(t, (Mu*)this), COMPACT);
                }
                s += ']';
                mess_cache = s;
            }
            else {
                mess_cache = '[' + get_demangled_name(cppt) + ']';
            }
        }
        return mess_cache.c_str();
    }
}

namespace in {
    void unrecoverable_exception (std::exception& e, Str when) {
        std::cerr << "Unrecoverable exception " << when
                  << ": " << e.what() << std::endl;
        std::abort();
    }
    void internal_error (
        const char* function, const char* filename, uint line
    ) {
        std::cerr << "Internal error in " << function
                  << " at " << filename << ":" << line << std::endl;
        std::abort();
    }
}

} using namespace ayu;

AYU_DESCRIBE(ayu::X::GenericError,
    elems( elem(&X::GenericError::mess) )
)
 // TODO: Use attrs instead of elems
AYU_DESCRIBE(ayu::X::OpenFailed,
    elems(
        elem(&X::OpenFailed::filename),
        elem(&X::OpenFailed::errnum)
    )
)
AYU_DESCRIBE(ayu::X::CloseFailed,
    elems(
        elem(&X::CloseFailed::filename),
        elem(&X::CloseFailed::errnum)
    )
)
