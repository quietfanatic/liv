// This module contains the meat of the serialization functionality of this
// library, implementing algorithms to transform objects to and from trees,
// based on the information in their descriptions.

#pragma once

#include "common.h"
#include "location.h"
#include "print.h"
#include "tree.h"
#include "type.h"

namespace ayu {

////// MAIN OPERATIONS
 // Convert an item to a tree.
Tree item_to_tree (const Reference&);
 // Write to an item from a tree.  If an error occurs, will leave the item in an
 // incomplete state.
void item_from_tree (const Reference&, const Tree&);

///// MAIN OPERATION SHORTCUTS
String item_to_string (const Reference&, PrintFlags flags = PrintFlags(0));
void item_to_file (const Reference&, Str filename, PrintFlags flags = PrintFlags(0));

void item_from_string (const Reference&, Str src);
void item_from_file (const Reference&, Str filename);

///// ACCESS OPERATIONS
 // Get a list of the keys in a object-like item
std::vector<String> item_get_keys (const Reference&);
 // Set the keys in an object-like item.  This may clear the entire contents
 // of the item.
void item_set_keys (const Reference&, const std::vector<String>&);
 // Get an attribute of an object-like item by its key, or empty Reference if
 // the attribute doesn't exist
Reference item_maybe_attr (const Reference&, Str);
 // Throws if the attribute doesn't exist
Reference item_attr (const Reference&, Str);

 // Get the length of an array-like item.
usize item_get_length (const Reference&);
 // Set the length of an array-like item.  This may clear some or all of the
 // contents of the item.
void item_set_length (const Reference&, usize);
 // Get an element of an array-like item by its index.  Returns an empty
 // Reference if te element doesn't exist.
Reference item_maybe_elem (const Reference&, usize);
 // Throws if the element doesn't exist
Reference item_elem (const Reference&, usize);

///// LOCATION OPERATIONS
 // Convert a Location to a Reference.  This will not have to do any scanning,
 // so it should be fairly quick.  Well, quicker than reference_to_location.
Reference reference_from_location (Location);

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

 // This is used by reference_to_location and KeepLocationCache.  You shouldn't
 // have to use this directly, but you can if you want.  This will scan all data
 // visible to ayu.  The callback will be called with:
 //   1. A reference to the currently scanned item
 //   2. The location of the currently scanned item
void recursive_scan_universe (
    Callback<void(const Reference&, Location)> cb
);

 // Scan only a particular resource.  Silently does nothing if the resource is
 // UNLOADED.  TODO: Should it throw instead?
void recursive_scan_resource (
    Resource res,
    Callback<void(const Reference&, Location)> cb
);

 // Scan only data under a given reference.  base_location should be the
 // location of base_item.
void recursive_scan (
    const Reference& base_item,
    Location base_location,
    Callback<void(const Reference&, Location)> cb
);

///// DIAGNOSTICS HELP

 // While this object is alive, if an exception is thrown while serializing an
 // item (and that exception is described to AYU), then the exception will be
 // caught and reported inline in the serialized output.  It will be in a format
 // that is not valid to read back in, so only use it for debugging.
 // Internally, this is used when generating the .what() message for exceptions.
struct DiagnosticSerialization {
    DiagnosticSerialization ();
    ~DiagnosticSerialization ();
};

} // namespace ayu

#include "reference.h"

namespace ayu::X {
     // Generic serialization error
    struct SerError : LogicError {
        Location location;
        String mess;
        SerError (const Reference& item);
        SerError (Location&& loc) : location(std::move(loc)) { }
        SerError (String&& mess) : mess(std::move(mess)) { }
    };
     // Tried to call to_tree on a type that doesn't support to_tree
    struct CannotToTree : SerError {
        using SerError::SerError;
    };
     // Tried to call from_tree on a type that doesn't support from_tree
    struct CannotFromTree : SerError {
        using SerError::SerError;
    };
     // Tried to deserialize an item from a tree, but the item didn't accept
     // the tree's form.
    struct InvalidForm : SerError {
        Tree tree;
        InvalidForm (const Reference& r, Tree t) : SerError(r), tree(t) { }
    };
     // Tried to serialize an item using a values() descriptor, but no value()
     // entry was found for the item's current value.
    struct NoNameForValue : SerError {
        using SerError::SerError;
    };
     // Tried to deserialize an item using a values() descriptor, but no value()
     // entry was found that matched the provided name.
    struct NoValueForName : SerError {
        Tree tree;
        NoValueForName (const Reference& r, Tree t) : SerError(r), tree(t) { }
    };
     // Tried to deserialize an item from an object tree, but the tree is
     // an attribute that the item requires.
    struct MissingAttr : SerError {
        String key;
        MissingAttr (const Reference& r, Str k) : SerError(r), key(k) { }
    };
     // Tried to deserialize an item from an object tree, but the item rejected
     // one of the attributes in the tree.
    struct UnwantedAttr : SerError {
        String key;
        UnwantedAttr (const Reference& r, Str k) : SerError(r), key(k) { }
    };
     // Tried to deserialize an item from an array tree, but the item didn't
     // the array's length.
    struct WrongLength : SerError {
        usize min;
        usize max;
        usize got;
        WrongLength (const Reference& r, usize a, usize b, usize g) :
            SerError(r), min(a), max(b), got(g)
        { }
    };
     // Tried to treat an item like it has attributes, but it does not support
     // behaving like an object.
    struct NoAttrs : SerError {
        using SerError::SerError;
    };
     // Tried to treat an item like it has elements, but it does not support
     // behaving like an array.
    struct NoElems : SerError {
        using SerError::SerError;
    };
     // Tried to get an attribute from an item, but it doesn't have an attribute
     // with the given key.
    struct AttrNotFound : SerError {
        String key;
        AttrNotFound (const Reference& r, Str k) : SerError(r), key(k) { }
    };
     // Tried to get an element from an item, but it doesn't have an element
     // with the given index.
    struct ElemNotFound : SerError {
        usize index;
        ElemNotFound (const Reference& r, usize i) : SerError(r), index(i) { }
    };
     // Tried to transform a Reference into a path, but a global scan could not
     // find where the Reference pointed to.
    struct UnresolvedReference : LogicError {
        Type type;
        UnresolvedReference (const Reference& r) : type(r.type()) { }
    };
}
