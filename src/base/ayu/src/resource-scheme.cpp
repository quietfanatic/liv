#include "../resource-scheme.h"

#include <cassert>
#include "../describe.h"
#include "char-cases-private.h"
#include "universe-private.h"

namespace ayu {
using namespace in;

void ResourceScheme::activate () const {
    auto& schemes = universe().schemes;
     // Easiest way to validate is just try creating an IRI
    if (!IRI(cat(scheme_name, ":"))) {
        throw X<InvalidResourceScheme>(scheme_name);
    }
    auto [iter, emplaced] = schemes.emplace(scheme_name, this);
    if (!emplaced) throw X<DuplicateResourceScheme>(scheme_name);
}
void ResourceScheme::deactivate () const {
    auto& schemes = universe().schemes;
    schemes.erase(scheme_name);
}

} using namespace ayu;

///// AYU DESCRIPTIONS

AYU_DESCRIBE(ayu::ResourceNameError,
    delegate(base<Error>())
)
AYU_DESCRIBE(ayu::InvalidResourceName,
    elems(
        elem(base<ResourceNameError>(), inherit),
        elem(&InvalidResourceName::name)
    )
)
AYU_DESCRIBE(ayu::UnknownResourceScheme,
    elems(
        elem(base<ResourceNameError>(), inherit),
        elem(&UnknownResourceScheme::name)
    )
)
AYU_DESCRIBE(ayu::UnacceptableResourceName,
    elems(
        elem(base<ResourceNameError>(), inherit),
        elem(&UnacceptableResourceName::name)
    )
)
AYU_DESCRIBE(ayu::UnacceptableResourceType,
    elems(
        elem(base<ResourceNameError>(), inherit),
        elem(&UnacceptableResourceType::name),
        elem(&UnacceptableResourceType::type)
    )
)
AYU_DESCRIBE(ayu::InvalidResourceScheme,
    elems(
        elem(base<ResourceNameError>(), inherit),
        elem(&InvalidResourceScheme::scheme)
    )
)
AYU_DESCRIBE(ayu::DuplicateResourceScheme,
    elems(
        elem(base<ResourceNameError>(), inherit),
        elem(&DuplicateResourceScheme::scheme)
    )
)

