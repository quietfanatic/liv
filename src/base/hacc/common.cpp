#include "common.h"

#include <cstdlib>
#include <iostream>

#include "haccable.h"
#include "serialize.h"

using namespace std::literals;

namespace hacc {
using namespace in;

void dump_ref (const Reference& r) {
    std::cerr << item_to_string(r) << std::flush;
}

namespace X {
    const char* Error::what () const noexcept {
        if (mess_cache.empty()) {
            auto& cppt = typeid(*this);
            if (Type t = get_description_by_type_info(cppt)) {
                String s = "["s + t.name() + ' ';
                try {
                     // TODO: this is INCORRECT and will BREAK without downcast
                    s += item_to_string(Reference(t, (Mu*)this), COMPACT);
                }
                catch (std::exception&) {
                    s += "(Another error occurred while serializing the error data)"s;
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
        std::cerr << "Unrecoverable exception " << when << ": " << e.what() << std::endl;
        std::abort();
    }
    void internal_error (const char* function, const char* filename, uint line) {
        std::cerr << "Internal error in " << function << " at " << filename << ":" << line << std::endl;
        std::abort();
    }
}

} using namespace hacc;


HACCABLE(hacc::X::GenericError,
    elems( elem(&X::GenericError::mess) )
)
HACCABLE(hacc::X::OpenFailed,
    elems(
        elem(&X::OpenFailed::filename),
        elem(&X::OpenFailed::errnum)
    )
)
HACCABLE(hacc::X::CloseFailed,
    elems(
        elem(&X::CloseFailed::filename),
        elem(&X::CloseFailed::errnum)
    )
)