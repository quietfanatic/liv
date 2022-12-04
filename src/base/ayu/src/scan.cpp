#include "../scan.h"

#include "../location.h"
#include "../reference.h"
#include "../serialize.h"
#include "descriptors-private.h"
#include "resource-private.h"

namespace ayu {
namespace in {
    static std::unordered_map<Reference, Location> location_cache;
    static usize keep_location_count = 0;
} using namespace in;

KeepLocationCache::KeepLocationCache () {
    keep_location_count++;
}
KeepLocationCache::~KeepLocationCache () {
    if (!--keep_location_count) {
        location_cache.clear();
    }
}

Location reference_to_location (const Reference& ref) {
    KeepLocationCache keep;
    if (location_cache.empty()) {
        recursive_scan_universe(
            [](const Reference& ref, Location loc){
                location_cache.emplace(ref, loc);
            }
        );
    }
    auto it = location_cache.find(ref);
    if (it != location_cache.end()) return it->second;
    else throw X::UnresolvedReference(ref);
}

String show_reference (const Reference& ref) {
    try {
        Location loc = reference_to_location(ref);
        return item_to_string(&loc);
    }
    catch (std::exception& e) {
        return cat("(An error occurred while showing this reference: "sv, e.what(), ')');
    }
}

void recursive_scan_universe (
    Callback<void(const Reference&, Location)> cb
) {
    for (auto& [_, resdat] : universe().resources) {
        recursive_scan_resource(Resource(&*resdat), cb);
    }
}

void recursive_scan_resource (
    const Resource& res,
    Callback<void(const Reference&, Location)> cb
) {
    if (res.state() == UNLOADED) return;
    recursive_scan(res.get_value(), Location(res), cb);
}

 // TODO: Skip atomic types T if AYU_DESCRIBE for T* has not been instantiated
void recursive_scan (
    const Reference& item, const Location& loc,
    Callback<void(const Reference&, Location)> cb
) {
    if (!item) return;
    cb(item, loc);

    auto desc = DescriptionPrivate::get(item.type());
    switch (desc->preference()) {
        case Description::PREFER_OBJECT: {
            for (auto& k : item_get_keys(item)) {
                recursive_scan(item_attr(item, k), Location(loc, k), cb);
            }
            return;
        }
        case Description::PREFER_ARRAY: {
            usize l = item_get_length(item);
            for (usize i = 0; i < l; i++) {
                recursive_scan(item_elem(item, i), Location(loc, i), cb);
            }
            return;
        }
        default: {
            if (auto acr = desc->delegate_acr()) {
                recursive_scan(item.chain(acr), loc, cb);
            }
            return;
        }
    }
}

} // namespace ayu
