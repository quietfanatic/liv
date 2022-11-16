// A Location is an intermediate step between a Reference and an IRI.  A valid
// Location can be easily converted to and from a valid IRI.  A Location can
// also be easily converted to a Reference, but converting a Reference to a
// Location may require scanning a lot of data.  The functions for doing these
// conversions are in serialize.h.
//
// You shouldn't have to use this class directly, but I guess you can if you
// want to.
//
// Internally, a Location is a recursive object that is a symbolic
// representation of a Reference, explaining how to reach the referend from the
// root Resource by a chain of item_attr() and item_elem() calls. In ADT syntax,
//     data Location = RootLocation Resource
//                   | KeyLocation Location String
//                   | IndexLocation Location usize
//
// TODO: Provide functions to translate References directly to and from IRIs
// somewhere.

#pragma once

#include "internal/common-internal.h"

namespace ayu {

 // Locations are a reference-counted pointer, so are cheap to copy.  Locations are
 // immutable once created.
struct Location {
    in::RCP<in::LocationData, in::delete_LocationData> data;
    constexpr explicit Location (in::LocationData* p = null) : data(p) { }
     // Trying to do anything with the empty location will segfault.
    explicit operator bool () const { return !!data; }
     // Constructs a root location from a Resource.
    explicit Location (Resource);
     // Constructs a location based on another one with an added attribute key
     // or element index.
    Location (Location parent, String&& key);
    Location (Location parent, Str key) : Location(parent, String(key)) { }
    Location (Location parent, usize index);
     // Parses an IRI into a location.  All of the IRI up to the fragment will
     // be used as the resource name for the root, and the fragment will be
     // split on / and each segment used as either a key or index.  To force a
     // string of digits to be used as a key instead of an index, precede it
     // with ' (apostrophe).  To start a key with a literal ', start it
     // with two 's.  To put a literal / in a key, use %2F.
    explicit Location (const IRI& iri);

     // Returns this location in IRI form
    IRI as_iri () const;

     // Returns null if this is not a root.
    const Resource* resource () const;
     // Returns null if this is a root.
    const Location* parent () const;
     // Returns null if this location is a root or has an index.
    const String* key () const;
     // Returns null if this location is a root or has a key.
    const usize* index () const;
     // Returns 1 for root, plus 1 for every key or index in the list.
    usize length () const;
};

bool operator == (const Location& a, const Location& b);
inline bool operator != (const Location& a, const Location& b) {
    return !(a == b);
}

 // Functions for converting locations to and from references are in serialize.h

} // namespace ayu
