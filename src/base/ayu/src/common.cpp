#include "../internal/common-internal.h"

#include <cstdlib>
#include <iostream>

#include "../compat.h"
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

namespace X {
    const char* Error::what () const noexcept {
        if (mess_cache.empty()) {
            auto& cppt = typeid(*this);
            if (Type t = get_description_by_type_info(cppt)) {
                String s = cat('[', t.name(), ' ');
                {
                    DiagnosticSerialization ds;
                    if (auto derived = Reference(this).try_downcast_to(t)) {
                        s += item_to_string(derived);
                    }
                    else {
                        s += "?(Could not downcast error data)"sv;
                    }
                }
                s += ']';
                mess_cache = s;
            }
            else {
                mess_cache = cat('[', get_demangled_name(cppt), ']');
            }
        }
        return mess_cache.c_str();
    }
}

namespace in {
    void unrecoverable_exception (std::exception& e, Str when) {
        std::cerr << "Unrecoverable exception "sv << when
                  << ": "sv << e.what() << std::endl;
        std::abort();
    }
    void internal_error (
        const char* function, const char* filename, uint line
    ) {
        std::cerr << "Internal error in "sv << function
                  << " at "sv << filename << ":"sv << line << std::endl;
        std::abort();
    }
}

} using namespace ayu;

AYU_DESCRIBE(ayu::X::Error,
    elems(),
    attrs()
)

AYU_DESCRIBE(ayu::X::GenericError,
    elems(
        elem(base<X::Error>(), inherit),
        elem(&X::GenericError::mess)
    )
)
AYU_DESCRIBE(ayu::X::IOError,
    elems(
        elem(base<X::Error>(), inherit),
        elem(&X::IOError::filename),
        elem(&X::IOError::errnum)
    )
)
AYU_DESCRIBE(ayu::X::OpenFailed,
    elems(elem(base<X::IOError>(), inherit))
)
AYU_DESCRIBE(ayu::X::ReadFailed,
    elems(elem(base<X::IOError>(), inherit))
)
AYU_DESCRIBE(ayu::X::CloseFailed,
    elems(elem(base<X::IOError>(), inherit))
)
