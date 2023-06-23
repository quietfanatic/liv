#include "../scan.h"

#include "../describe.h"
#include "../location.h"
#include "../pointer.h"
#include "../reference.h"
#include "descriptors-private.h"
#include "serialize-private.h"
#include "traversal-private.h"
#include "universe-private.h"

namespace ayu {
namespace in {

bool scan_trav (
    const Traversal& trav, LocationRef loc,
    CallbackRef<bool(const Traversal&, LocationRef)> cb
) {
    if (cb(trav, loc)) return true;
    switch (trav.desc->preference()) {
        case Description::PREFER_OBJECT: {
            UniqueArray<AnyString> ks;
            ser_collect_keys(trav, ks);
            for (auto& k : ks) {
                 // Initialize to false because in only_addressable mode, the
                 // callback may not be called.
                bool r = false;
                 // TODO: Avoid string copy
                ser_attr(trav, OldStr(k), ACR_READ, [&](const Traversal& child){
                    r = scan_trav(child, Location(loc, OldStr(k)), cb);
                });
                if (r) return true;
            }
            return false;
        }
        case Description::PREFER_ARRAY: {
            usize len = ser_get_length(trav);
            for (usize i = 0; i < len; i++) {
                bool r = false;
                ser_elem(trav, i, ACR_READ, [&](const Traversal& child){
                    r = scan_trav(child, Location(loc, i), cb);
                });
                if (r) return true;
            }
            return false;
        }
        default: {
            if (auto acr = trav.desc->delegate_acr()) {
                bool r = false;
                trav_delegate(
                    trav, acr, ACR_READ, [&](const Traversal& child)
                {
                    r = scan_trav(child, loc, cb);
                });
                return r;
            }
            return false;
        }
    }
}

 // Store a typed Pointer instead of a Mu* because items at the same address
 // with different types are different items.
static std::unordered_map<Pointer, Location> location_cache;
static bool have_location_cache = false;
static usize keep_location_cache_count = 0;
std::unordered_map<Pointer, Location>* get_location_cache () {
    if (!keep_location_cache_count) return null;
    if (!have_location_cache) {
        scan_universe_pointers([&](Pointer ptr, LocationRef loc){
            location_cache.emplace(ptr, loc);
            return false;
        });
        have_location_cache = true;
    }
    return &location_cache;
}

} using namespace in;

KeepLocationCache::KeepLocationCache () {
    keep_location_cache_count++;
}
KeepLocationCache::~KeepLocationCache () {
    if (!--keep_location_cache_count) {
        have_location_cache = false;
        location_cache.clear();
    }
}

bool scan_pointers (
    Pointer base_item, LocationRef base_loc,
    CallbackRef<bool(Pointer, LocationRef)> cb
) {
    bool r = false;
    trav_start(base_item, base_loc, true, ACR_READ, [&](const Traversal& trav){
        r = scan_trav(
            trav, base_loc, [&](const Traversal& trav, LocationRef loc)
        {
            if (trav.addressable) {
                return cb(Pointer(trav.address, trav.desc), loc);
            }
            else return false;
        });
    });
    return r;
}

bool scan_references (
    const Reference& base_item, LocationRef base_loc,
    CallbackRef<bool(const Reference&, LocationRef)> cb
) {
    bool r = false;
    trav_start(base_item, base_loc, false, ACR_READ, [&](const Traversal& trav){
        r = scan_trav(
            trav, base_loc, [&](const Traversal& trav, LocationRef loc)
        {
            return cb(trav_reference(trav), loc);
        });
    });
    return r;
}

bool scan_resource_pointers (
    const Resource& res, CallbackRef<bool(Pointer, LocationRef)> cb
) {
    if (res.state() == UNLOADED) return false;
    return scan_pointers(res.get_value().ptr(), Location(res), cb);
}
bool scan_resource_references (
    const Resource& res, CallbackRef<bool(const Reference&, LocationRef)> cb
) {
    if (res.state() == UNLOADED) return false;
    return scan_references(res.get_value().ptr(), Location(res), cb);
}

bool scan_universe_pointers (
    CallbackRef<bool(Pointer, LocationRef)> cb
) {
    for (auto& [_, resdat] : universe().resources) {
        if (scan_resource_pointers(&*resdat, cb)) return true;
    }
    return false;
}

bool scan_universe_references (
    CallbackRef<bool(const Reference&, LocationRef)> cb
) {
    for (auto& [_, resdat] : universe().resources) {
        if (scan_resource_references(&*resdat, cb)) return true;
    }
    return false;
}

Location find_pointer (Pointer item) {
    if (!item) return Location();
    else if (auto cache = get_location_cache()) {
        auto it = cache->find(item);
        if (it != cache->end()) return it->second;
        else return Location();
    }
    else {
        Location r;
        scan_universe_pointers([&](Pointer p, LocationRef loc){
            if (p == item) {
                r = loc;
                return true;
            }
            else return false;
        });
        return r;
    }
}

Location find_reference (const Reference& item) {
    if (!item) return Location();
    else if (auto cache = get_location_cache()) {
        if (Mu* address = item.address()) {
             // Addressable! This will be fast.
            auto it = cache->find(Pointer(address, item.type()));
            if (it != cache->end()) return it->second;
            else return Location();
        }
        else {
             // Not addressable.  First find the host in the location cache.
            auto it = cache->find(item.host);
            if (it != cache->end()) {
                 // Now search under that host for the actual reference.
                 // This will likely fail because it's hard to compare
                 // unaddressable references, but try anyway.
                Location r;
                scan_references(
                    Reference(item.host), it->second,
                    [&](const Reference& ref, LocationRef loc)
                {
                    if (ref == item) {
                        r = loc;
                        return true;
                    }
                    else return false;
                });
                return r;
            }
            else return Location();
        }
    }
    else {
         // We don't have the location cache!  Time to do a global search.
        Location r;
        scan_universe_references(
            [&](const Reference& ref, LocationRef loc)
        {
            if (ref == item) {
                r = loc;
                return true;
            }
            else return false;
        });
        return r;
    }
}

Location pointer_to_location (Pointer item) {
    if (!item) return Location();
    else if (Location r = find_pointer(item)) {
        return r;
    }
    else throw X<ReferenceNotFound>(item.type);
}
Location reference_to_location (const Reference& item) {
    if (!item) return Location();
    else if (Location r = find_reference(item)) {
        return r;
    }
    else throw X<ReferenceNotFound>(item.type());
}

} using namespace ayu;

AYU_DESCRIBE(ayu::ReferenceNotFound,
    elems(
        elem(base<Error>(), inherit),
        elem(&ReferenceNotFound::type)
    )
)
