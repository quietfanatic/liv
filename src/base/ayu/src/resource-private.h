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
        const ResourceScheme* require_scheme (const IRI& name) {
            Str scheme = name.scheme();
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

    inline void verify_tree_for_scheme (
        Resource res,
        const ResourceScheme* scheme,
        const Tree& tree
    ) {
        if (tree.form() == NULLFORM) {
            throw X::EmptyResourceValue(String(res.name().spec()));
        }
        const Array& array = Array(tree);
        if (array.size() == 2) {
            Type type = Type(Str(array[0]));
            if (!scheme->accepts_type(type)) {
                throw X::UnacceptableResourceType(
                    String(res.name().spec()), type
                );
            }
        }
    }

} using namespace in;
} using namespace ayu;

