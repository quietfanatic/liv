// This module contains the meat of the serialization functionality of this
// library, implementing algorithms to transform objects to and from trees,
// based on the information in their descriptions.
//
// Serialization functions cannot be used until main() starts.

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
    const Reference&, LocationRef loc = Location()
);

 // Flags to change the behavior of item_from_tree.
using ItemFromTreeFlags = uint32;
enum : ItemFromTreeFlags {
     // If calling item_from_tree recursively, schedule swizzle and init
     // operations for after the outer call does its swizzle and init
     // operations respectively.  This will allow items to cyclically reference
     // one another, but can only be used if
     //   A: the provided reference will still be valid later on (e.g it's not
     //      the address of a stack temporary that's about to be moved into a
     //      container), and
     //   B: the item's treatment will not change based on its value.  For
     //      instance, this is not usable on the elements of a
     //      std::unordered_set, because the position of a set element depends
     //      on its value, and updating it in place without notifying the
     //      unordered_set would corrupt the unordered_set.
     // item_from_tree cannot check that these conditions are true, so if you
     // use this flag when they are not true, you will likely corrupt memory.
     //
     // For non-recursive item_from_tree calls, this flag has no effect.
    DELAY_SWIZZLE = 1,
};
 // Write to an item from a tree.  If an exception is thrown, the item may be
 // left in an incomplete state, so if you're worried about that, construct a
 // fresh item, call item_from_tree on that, and then move it onto the original
 // item (this is what ayu::reload() on resources does).
void item_from_tree (
    const Reference&, TreeRef, LocationRef loc = Location(),
    ItemFromTreeFlags flags = 0
);

///// MAIN OPERATION SHORTCUTS
inline std::string item_to_string (
    const Reference& item, PrintOptions opts = 0,
    LocationRef loc = Location()
) {
    return tree_to_string(item_to_tree(item, loc), opts);
}
inline void item_to_file (
    const Reference& item, Str filename,
    PrintOptions opts = 0, LocationRef loc = Location()
) {
    return tree_to_file(item_to_tree(item, loc), filename, opts);
}
 // item_from_string and item_from_file do not currently allow passing flags
inline void item_from_string (
    const Reference& item, Str src, LocationRef loc = Location()
) {
    return item_from_tree(item, tree_from_string(src), loc);
}
inline void item_from_file (
    const Reference& item, Str filename, LocationRef loc = Location()
) {
    return item_from_tree(item, tree_from_file(filename), loc);
}

///// ACCESS OPERATIONS
 // Get a list of the keys in a object-like item.
AnyArray<TreeString> item_get_keys (
    const Reference&, LocationRef loc = Location()
);
 // Set the keys in an object-like item.  This may clear the entire contents
 // of the item.
void item_set_keys (
    const Reference&, Slice<Str>,
    LocationRef loc = Location()
);
 // Get an attribute of an object-like item by its key, or empty Reference if
 // the attribute doesn't exist.
Reference item_maybe_attr (
    const Reference&, Str, LocationRef loc = Location());
 // Throws if the attribute doesn't exist.  Guaranteed not to return an empty or
 // null Reference.
Reference item_attr (const Reference&, Str, LocationRef loc = Location());

 // Get the length of an array-like item.
usize item_get_length (const Reference&, LocationRef loc = Location());
 // Set the length of an array-like item.  This may clear some or all of the
 // contents of the item.
void item_set_length (
    const Reference&, usize, LocationRef loc = Location()
);
 // Get an element of an array-like item by its index.  Returns an empty
 // Reference if te element doesn't exist.
Reference item_maybe_elem (
    const Reference&, usize, LocationRef loc = Location()
);
 // Throws if the element doesn't exist.  Guaranteed not to return an empty or
 // null Reference.
Reference item_elem (
    const Reference&, usize, LocationRef loc = Location()
);

///// MISC

 // If a serialization operation is active, get the Location of an item currently
 // being processed.
Location current_location ();

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

 // Generic serialization error
struct SerError : Error {
    Location location;
};
 // Tried to call to_tree on a type that doesn't support to_tree
struct CannotToTree : SerError { };
 // Tried to call from_tree on a type that doesn't support from_tree
struct CannotFromTree : SerError { };
 // Tried to deserialize an item from a tree, but the item didn't accept
 // the tree's form.
struct InvalidForm : SerError {
    Tree tree;
};
 // Tried to serialize an item using a values() descriptor, but no value()
 // entry was found for the item's current value.
struct NoNameForValue : SerError { };
 // Tried to deserialize an item using a values() descriptor, but no value()
 // entry was found that matched the provided name.
struct NoValueForName : SerError {
    Tree tree;
};
 // Tried to deserialize an item from an object tree, but the tree is
 // an attribute that the item requires.
struct MissingAttr : SerError {
    std::string key;
};
 // Tried to deserialize an item from an object tree, but the item rejected
 // one of the attributes in the tree.
struct UnwantedAttr : SerError {
    std::string key;
};
 // Tried to deserialize an item from an array tree, but the array has too
 // few or too many elements for the item.
struct WrongLength : SerError {
    usize min;
    usize max;
    usize got;
};
 // Tried to treat an item like it has attributes, but it does not support
 // behaving like an object.
struct NoAttrs : SerError { };
 // Tried to treat an item like it has elements, but it does not support
 // behaving like an array.
struct NoElems : SerError { };
 // Tried to get an attribute from an item, but it doesn't have an attribute
 // with the given key.
struct AttrNotFound : SerError {
    std::string key;
};
 // Tried to get an element from an item, but it doesn't have an element
 // with the given index.
struct ElemNotFound : SerError {
    usize index;
};
 // The accessor given to a keys() descriptor did not serialize to an array
 // of strings.
struct InvalidKeysType : SerError {
    Type type;
};

} // namespace ayu
