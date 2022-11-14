#include "../resource-scheme.h"

#include <cassert>
#include <vector>
#include "../describe.h"
#include "char-cases-private.h"
#include "resource-private.h"

namespace ayu {

void ResourceScheme::activate () const {
    auto& schemes = universe().schemes;
     // Validate
    if (scheme_name.size() == 0) {
        throw X::InvalidResourceScheme(String(scheme_name));
    }
    for (const char& c : scheme_name)
    switch (c) {
        case ANY_LOWERCASE: break;
        case ANY_UPPERCASE:
        case ANY_DECIMAL_DIGIT:
        case '+': case '-': case '.':
            if (&c == &scheme_name.front()) {
                throw X::InvalidResourceScheme(String(scheme_name));
            }
            else break;
    }
     // Register
    auto [iter, emplaced] = schemes.emplace(scheme_name, this);
    if (!emplaced) throw X::DuplicateResourceScheme(String(scheme_name));
}
void ResourceScheme::deactivate () const {
    auto& schemes = universe().schemes;
    schemes.erase(scheme_name);
}

} // namespace ayu

///// AYU DESCRIPTIONS

AYU_DESCRIBE(ayu::X::ResourceNameError,
    delegate(base<X::Error>())
)
AYU_DESCRIBE(ayu::X::InvalidResourceName,
    delegate(base<X::ResourceNameError>()),
    elems(elem(&X::InvalidResourceName::name))
)
AYU_DESCRIBE(ayu::X::UnknownResourceScheme,
    delegate(base<X::ResourceNameError>()),
    elems(elem(&X::UnknownResourceScheme::name))
)
AYU_DESCRIBE(ayu::X::UnacceptableResourceName,
    delegate(base<X::ResourceNameError>()),
    elems(elem(&X::UnacceptableResourceName::name))
)
AYU_DESCRIBE(ayu::X::UnacceptableResourceType,
    delegate(base<X::ResourceNameError>()),
    elems(
        elem(&X::UnacceptableResourceType::name),
        elem(&X::UnacceptableResourceType::type)
    )
)
AYU_DESCRIBE(ayu::X::InvalidResourceScheme,
    delegate(base<X::ResourceNameError>()),
    elems(elem(&X::InvalidResourceScheme::scheme))
)
AYU_DESCRIBE(ayu::X::DuplicateResourceScheme,
    delegate(base<X::ResourceNameError>()),
    elems(elem(&X::DuplicateResourceScheme::scheme))
)

