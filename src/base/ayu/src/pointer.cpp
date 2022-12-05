#include "../pointer.h"

#include "../describe.h"
#include "../reference.h"

namespace ayu {
} using namespace ayu;

AYU_DESCRIBE(ayu::Pointer,
    delegate(assignable<Reference>())
);
