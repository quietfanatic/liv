#include "../exception.h"

#include <iostream>
#include "../../uni/utf.h"
#include "../describe.h"
#include "../serialize.h"

namespace ayu {
using namespace in;

const char* ExceptionBase::what () const noexcept {
    if (mess_cache.empty()) {
        Pointer p = ptr();
        mess_cache = old_cat('[', p.type.name(), ' ', item_to_string(p), ']');
    }
    return mess_cache.c_str();
}

void in::unrecoverable_exception (std::exception& e, OldStr when) {
    std::cerr << "ERROR: Unrecoverable exception "sv << when
              << ": "sv << e.what() << std::endl;
    std::abort();
}

} using namespace ayu;

AYU_DESCRIBE(std::source_location,
    elems(
        elem(value_func<std::string>([](const std::source_location& v) -> std::string {
            return v.file_name();
        })),
        elem(value_func<std::string>([](const std::source_location& v) -> std::string {
            return v.function_name();
        })),
        elem(value_method<uint32, &std::source_location::line>()),
        elem(value_method<uint32, &std::source_location::column>())
    )
)
