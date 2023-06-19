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
//                   | KeyLocation Location std::string
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
     // The empty location is treated as the location of an anonymous item, and
     // can't be transformed into a reference.
    explicit operator bool () const { return !!data; }
     // Constructs a root location from a Resource.
    explicit Location (Resource);
     // Constructs a location based on another one with an added attribute key
     // or element index.  TODO: Take a Tree
    Location (LocationRef parent, std::string&& key);
    Location (LocationRef parent, OldStr key) : Location(parent, std::string(key)) { }
    Location (LocationRef parent, usize index);
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
    const std::string* key () const;
     // Returns null if this location is a root or has a key.
    const usize* index () const;
     // Returns 1 for root, plus 1 for every key or index in the list.
    usize length () const;

     // Walks all the way to the root and returns its Resource, if any
    const Resource* root_resource () const;
};

bool operator == (LocationRef a, LocationRef b);

 // Convert a Location to a Reference.  This will not have to do any scanning,
 // so it should be fairly quick.  Well, quicker than reference_to_location.
 // reference_to_location is in scan.h
Reference reference_from_location (Location);

} // namespace ayu
