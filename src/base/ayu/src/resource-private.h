#pragma once

#include "../common.h"
#include "../resource.h"
#include "../resource-scheme.h"

#include <unordered_map>

namespace ayu {
namespace in {

    struct Universe {
        std::unordered_map<Str, ResourceData*> resources;
        Resource current_resource;
        std::unordered_map<String, const ResourceScheme*> schemes;
        FileResourceScheme default_scheme {"file", "/", false};
        const ResourceScheme* require_scheme (Str scheme) {
            if (schemes.empty()) {
                if (scheme == "file") return &default_scheme;
                else throw X::UnknownResourceScheme(String(scheme));
            }
            else {
                auto iter = schemes.find(String(scheme));
                if (iter != schemes.end()) return iter->second;
                else throw X::UnknownResourceScheme(String(scheme));
            }
        }
        const ResourceScheme* scheme_for_iri (const IRI& name) {
            auto scheme = require_scheme(name.scheme());
            if (scheme->accepts(name)) return scheme;
            else throw X::UnacceptableResourceName(String(name.spec()));
        }
    };
    Universe& universe ();
     // TODO: Should this be public?
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

