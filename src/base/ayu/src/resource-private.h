#pragma once

#include <memory>
#include <unordered_map>
#include "../common.h"
#include "../resource.h"
#include "../resource-scheme.h"

namespace ayu {
namespace in {

    struct Universe {
        std::unordered_map<Str, std::unique_ptr<ResourceData>> resources;
        Resource current_resource;
        Reference current_base_item;
        Location current_base_location;
        std::unordered_map<String, const ResourceScheme*> schemes;
        FileResourceScheme default_scheme {"file"s, "/"s, false};
        const ResourceScheme* require_scheme (const IRI& name) {
            Str scheme = name.scheme();
            if (schemes.empty()) {
                if (scheme == "file"sv) return &default_scheme;
                else throw X::UnknownResourceScheme(String(scheme));
            }
            else {
                auto iter = schemes.find(String(scheme));
                if (iter != schemes.end()) return iter->second;
                else throw X::UnknownResourceScheme(String(scheme));
            }
        }
    };
    Universe& universe ();
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

