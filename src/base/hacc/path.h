#pragma once

// This implements a recursive object that is a symbolic representation of a
//  Reference.  I would have just used hacc::Array, but due to the way these
//  are used, a linked list structure will be much more efficient than a
//  packed array structure.
// TODO: Are we sure about that?  A hacc::Array is just a std::vector of
//  refcounted pointers.  Worth some benchmarking once we get around to it.
//
// You shouldn't have to use this class directly, but I guess you can if you
//  want.

#include "common.h"

namespace hacc {

 // Paths are a reference-counted pointer, so are cheap to copy.  Paths are
 //  immutable once created.
struct Path {
    in::RCP<in::PathData, in::delete_PathData> data;
     // Constructs a path to the root.
    Path () : data(null) { }
     // Constructs a path based on another one with an attribute key or an
     //  element index.
    Path (Path parent, String&& key);
    Path (Path parent, Str key) : Path(parent, String(key)) { }
    Path (Path parent, usize index);

    bool is_root () const { return !data; }
     // Returns null if the path is root.
    const Path* parent () const;
     // Returns null if the path is root or has an index.
    const String* key () const;
     // Returns null if the path is root or has a key.
    const usize* index () const;
     // Returns 0 for root, plus 1 for every key or index in the path.
    usize length () const;
};

bool operator == (const Path& a, const Path& b);
static bool operator != (const Path& a, const Path& b) { return !(a == b); }

 // Functions for converting paths to and from references are in serialize.h

} // namespace hacc
