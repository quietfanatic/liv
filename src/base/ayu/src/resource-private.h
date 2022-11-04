#pragma once

#include "../common.h"
#include "../resource.h"
#include "../resource-name.h"

#include <unordered_map>

namespace ayu {
namespace in {

    struct Universe {
        std::unordered_map<Str, ResourceData*> resources;
        Resource current_resource;
        std::unordered_map<String, const ResourceScheme*> schemes;
        FileResourceScheme default_scheme {"file", "/", false};
    };
    Universe& universe ();
     // TODO: Should this be external?
    inline Reference universe_ref () { return &universe(); }
    struct PushCurrentResource {
        Resource old_current;
        PushCurrentResource (Resource res) :
            old_current(universe().current_resource)
        {
            universe().current_resource = res;
        }
        ~PushCurrentResource () {
            universe().current_resource = old_current;
        }
        PushCurrentResource (const PushCurrentResource&) = delete;
    };

} using namespace in;
} using namespace ayu;

