// This implements a recursive object that is a symbolic representation of a
// Reference, explaining how to reach the referend from the root.  I would have
// just used ayu::Array, but due to the way these are used, a linked list
// structure will be much more efficient than a packed array structure.
// TODO: Are we sure about that?  An ayu::Array is just a std::vector of
// refcounted pointers.  Worth some benchmarking once we get around to it.
//
// The empty Location refers to the root, known internally as the Universe.
// The first step after the root should always be a string key, and that key
// will be a resource name.  Keys and indexes after that will point to the
// contents of that resource.
//
// You shouldn't have to use this class directly, but I guess you can if you
// want to.

#pragma once

#include "internal/common-internal.h"

namespace ayu {

 // Locations are a reference-counted pointer, so are cheap to copy.  Locations are
 // immutable once created.
struct Location {
    in::RCP<in::LocationData, in::delete_LocationData> data;
     // Constructs the root location.
    Location () : data(null) { }
     // Constructs a location based on another one with an added attribute key
     // or element index.
    Location (Location parent, String&& key);
    Location (Location parent, Str key) : Location(parent, String(key)) { }
    Location (Location parent, usize index);

    bool is_root () const { return !data; }
     // Returns null if is_root.
    const Location* parent () const;
     // Returns null if this location is root or has an index.
    const String* key () const;
     // Returns null if this location is root or has a key.
    const usize* index () const;
     // Returns 0 for root, plus 1 for every key or index in the list.
    usize length () const;
};

bool operator == (const Location& a, const Location& b);
inline bool operator != (const Location& a, const Location& b) {
    return !(a == b);
}

 // Functions for converting locations to and from references are in serialize.h

} // namespace ayu
