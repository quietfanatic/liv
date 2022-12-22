#include "common.h"

#include "../ayu/describe.h"

namespace uni {

void assert_failed_general (const char* function, const char* filename, uint line) {
    throw ayu::X<AssertionFailed>(function, filename, line);
}

} using namespace uni;

AYU_DESCRIBE(uni::AssertionFailed,
    delegate(base<ayu::Error>()),
    elems(
        elem(&AssertionFailed::function),
        elem(&AssertionFailed::filename),
        elem(&AssertionFailed::line)
    )
)
