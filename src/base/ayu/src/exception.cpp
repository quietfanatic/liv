#include "../exception.h"

#include <iostream>
#include "../compat.h"
#include "../describe.h"
#include "../serialize.h"

namespace ayu {
using namespace in;

const char* ExceptionBase::what () const noexcept {
    if (mess_cache.empty()) {
        Pointer p = ptr();
        mess_cache = cat('[', p.type.name(), ' ', item_to_string(p), ']');
    }
    return mess_cache.c_str();
}

void in::unrecoverable_exception (std::exception& e, Str when) {
    std::cerr << "Unrecoverable exception "sv << when
              << ": "sv << e.what() << std::endl;
    std::abort();
}
void in::internal_error (std::source_location loc) {
    std::cerr << "Internal error in "sv << loc.function_name()
              << " at "sv << loc.file_name()
              << ":"sv << loc.line() << std::endl;
    std::abort();
}

} using namespace ayu;
