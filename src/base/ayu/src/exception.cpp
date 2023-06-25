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
        mess_cache = cat('[', p.type.name(), ' ', item_to_string(p), "]\0");
    }
    return mess_cache.data();
}

void in::unrecoverable_exception (std::exception& e, Str when) {
    warn_utf8(cat("ERROR: Unrecoverable exception ", when, ": ", e.what(), '\n'));
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
