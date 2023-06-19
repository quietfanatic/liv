 // The "Universe" manages the set of loaded resources and related global data.

#pragma once

#include <memory>
#include <unordered_map>
#include "../common.h"
#include "../resource.h"
#include "../resource-scheme.h"

namespace ayu::in {
    struct ResourceData {
        IRI name;
        Dynamic value {};
        Dynamic old_value {};  // Used when reloading
        ResourceState state = UNLOADED;
    };

    struct Universe {
        std::unordered_map<OldStr, std::unique_ptr<ResourceData>> resources;
        std::unordered_map<std::string, const ResourceScheme*> schemes;
        const ResourceScheme* require_scheme (const IRI& name) {
            OldStr scheme = name.scheme();
            auto iter = schemes.find(std::string(scheme));
            if (iter != schemes.end()) return iter->second;
            else throw X<UnknownResourceScheme>(std::string(scheme));
        }
    };

    inline Universe& universe () {
        static Universe r;
        return r;
    }

}

