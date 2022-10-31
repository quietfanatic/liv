// This module contains the meat of the serialization functionality of this
// library, implementing algorithms to transform objects to and from trees,
// based on the information in their descriptions.

#pragma once

#include "common.h"
#include "path.h"
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

///// PATH OPERATIONS
 // Convert a path to a reference.  This will not have to do any scanning,
 // so it should be fairly quick.
Reference reference_from_path (Path);

 // Convert a reference to a path.  This will be slow by itself, since it must
 // scan all loaded resources.  If a KeepPathCache object is alive, the first
 // call to reference_to_path will build a mapping References to Paths, and
 // subsequent calls to reference_to_path will be very fast.
Path reference_to_path (const Reference&);

 // Converts a reference to a string using reference_to_path.  If a
 // std::exception happens, will catch it and return a message containing its
 // what().  Intended for use in error message generation functions.
String show_reference (const Reference&);

 // While this is alive, a cache mapping references to paths will be kept,
 // making reference_to_path faster.  Do not modify any resource data while
 // keeping the path cache, since there is no way for the cache to stay
 // up-to-date.
struct KeepPathCache {
    KeepPathCache ();
    ~KeepPathCache ();
};

 // This is used by reference_to_path and KeepPathCache.  You shouldn't have to
 // use this directly, but you can if you want.  This will scan the given
 // reference and call the callback for every item in it.
 //   - item: The Reference to scan.
 //   - base: A path corresponding to item
 //   - cb: A callback called with:
 //       1. A reference to the currently scanned item
 //       2. A path to the currently scanned item
void recursive_scan (
    const Reference& item,
    Path base,
    Callback<void(const Reference&, Path)> cb
);

} // namespace ayu

#include "reference.h"

namespace ayu::X {
     // Generic serialization error
    struct SerError : LogicError {
        Path path_to_item;
        SerError (const Reference& item);
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