// This module contains the meat of the serialization functionality of this
// library, implementing algorithms to transform objects to and from trees,
// based on the information in their descriptions.

#pragma once

#include "common.h"
#include "location.h"
#include "parse.h"
#include "print.h"
#include "tree.h"
#include "type.h"

namespace ayu {

////// MAIN OPERATIONS
 // Convert an item to a tree.  The optional location should match the
 // reference's location if provided.
Tree item_to_tree (
    const Reference&, const Location& loc = Location()
);
 // Write to an item from a tree.  If an exception is thrown, the item may be
 // left in an incomplete state, so if you're worried about that, construct a
 // fresh item, call item_from_tree on that, and then move it onto the original
 // item (this is what ayu::reload() on resources does).
void item_from_tree (
    const Reference&, const Tree&, const Location& loc = Location()
);

///// MAIN OPERATION SHORTCUTS
inline String item_to_string (
    const Reference& item, PrintOptions opts = 0,
    const Location& loc = Location()
) {
    return tree_to_string(item_to_tree(item, loc), opts);
}
inline void item_to_file (
    const Reference& item, Str filename, PrintOptions opts = 0,
    const Location& loc = Location()
) {
    return tree_to_file(item_to_tree(item, loc), filename, opts);
}
inline void item_from_string (
    const Reference& item, Str src, const Location& loc = Location()
) {
    return item_from_tree(item, tree_from_string(src), loc);
}
inline void item_from_file (
    const Reference& item, Str filename, const Location& loc = Location()
) {
    return item_from_tree(item, tree_from_file(filename), loc);
}

///// ACCESS OPERATIONS
 // Get a list of the keys in an object-like item and pass them to a callback.
 // The Strs might not outlive the callback, so if you need to keep them around,
 // copy them or use item_get_keys instead.
void item_read_keys (
    const Reference&,
    Callback<void(const std::vector<Str>&)> cb,
    const Location& loc = Location()
);
 // Get a list of the keys in a object-like item.  This will copy all the
 // strings, so if you're concerned about performance, use item_read_keys
 // instead.
std::vector<String> item_get_keys (
    const Reference&, const Location& loc = Location()
);
 // Set the keys in an object-like item.  This may clear the entire contents
 // of the item.
void item_set_keys (
    const Reference&, const std::vector<Str>&,
    const Location& loc = Location()
);
 // Get an attribute of an object-like item by its key, or empty Reference if
 // the attribute doesn't exist.
Reference item_maybe_attr (
    const Reference&, Str, const Location& loc = Location());
 // Throws if the attribute doesn't exist.
Reference item_attr (const Reference&, Str, const Location& loc = Location());

 // Get the length of an array-like item.
usize item_get_length (const Reference&, const Location& loc = Location());
 // Set the length of an array-like item.  This may clear some or all of the
 // contents of the item.
void item_set_length (
    const Reference&, usize, const Location& loc = Location()
);
 // Get an element of an array-like item by its index.  Returns an empty
 // Reference if te element doesn't exist.
Reference item_maybe_elem (
    const Reference&, usize, const Location& loc = Location()
);
 // Throws if the element doesn't exist.
Reference item_elem (
    const Reference&, usize, const Location& loc = Location()
);

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
    struct SerError : Error {
        Location location;
        SerError (const Reference& item);
        SerError (Location&& loc) : location(std::move(loc)) { }
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
        InvalidForm (Location&& l, Tree t) : SerError(std::move(l)), tree(t) { }
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
        NoValueForName (Location&& l, Tree t) : SerError(std::move(l)), tree(t) { }
    };
     // Tried to deserialize an item from an object tree, but the tree is
     // an attribute that the item requires.
    struct MissingAttr : SerError {
        String key;
        MissingAttr (Location&& l, Str k) : SerError(std::move(l)), key(k) { }
    };
     // Tried to deserialize an item from an object tree, but the item rejected
     // one of the attributes in the tree.
    struct UnwantedAttr : SerError {
        String key;
        UnwantedAttr (Location&& l, Str k) : SerError(std::move(l)), key(k) { }
    };
     // Tried to deserialize an item from an array tree, but the array has too
     // few or too many elements for the item.
    struct WrongLength : SerError {
        usize min;
        usize max;
        usize got;
        WrongLength (Location&& l, usize min, usize max, usize g) :
            SerError(std::move(l)), min(min), max(max), got(g)
        { }
    };
     // Tried to deserialize an item from an array tree, but the array has too
     // many elements for the item.
    struct TooLong : SerError {
        usize max;
        usize got;
        TooLong (Location&& l, usize m, usize g) :
            SerError(std::move(l)), max(m), got(g)
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
        AttrNotFound (Location&& l, Str k) : SerError(std::move(l)), key(k) { }
    };
     // Tried to get an element from an item, but it doesn't have an element
     // with the given index.
    struct ElemNotFound : SerError {
        usize index;
        ElemNotFound (Location&& l, usize i) : SerError(std::move(l)), index(i) { }
    };
     // The accessor given to a keys() descriptor did not serialize to an array
     // of strings.
    struct InvalidKeysType : SerError {
        Type type;
        InvalidKeysType (Location&& l, Type t) : SerError(std::move(l)), type(t) { }
    };
     // Tried to transform a Reference into a path, but a global scan could not
     // find where the Reference pointed to.
    struct UnresolvedReference : SerError {
        Type type;
        UnresolvedReference (const Reference& r) : SerError(Location()), type(r.type()) { }
    };
}
