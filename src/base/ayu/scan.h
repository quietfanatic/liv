// This file contains operations that might require scanning large amounts of
// program data.

#pragma once
#include "common.h"

namespace ayu {

 // Convert a Reference to a Location.  This will be slow by itself, since it
 // must scan all loaded resources.  If a KeepLocationCache object is alive, the
 // first call to reference_to_location will build a map of References to
 // Locations, and subsequent calls to reference_to_location will be very fast.
Location reference_to_location (const Reference&);

 // While this is alive, a cache mapping references to locations will be kept,
 // making reference_to_location faster.  Do not modify any resource data while
 // keeping the location cache, since there is no way for the cache to stay
 // up-to-date.
struct KeepLocationCache {
    KeepLocationCache ();
    ~KeepLocationCache ();
};

 ///// Scanning operations

 // Scan all data visible to ayu.  This will be replaced soon.
void recursive_scan_universe (
    Callback<void(const Reference&, Location)> cb
);

 // Scan only a particular resource.  Silently does nothing if the resource is
 // UNLOADED.  TODO: Should it throw instead?
void recursive_scan_resource (
    const Resource& res,
    Callback<void(const Reference&, Location)> cb
);

 // Scan only data under a given reference.  base_location should be the
 // location of base_item.
void recursive_scan (
    const Reference& base_item,
    const Location& base_location,
    Callback<void(const Reference&, Location)> cb
);

} // namespace ayu
