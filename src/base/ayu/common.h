// This module contains various types and exceptions that are used throughout
// the library.

#pragma once

#include <cstdint>
#include <cwchar>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "../uni/arrays.h"
#include "../uni/callback.h"
#include "../uni/common.h"
#include "../uni/copy-ref.h"
#include "../uni/requirements.h"
#include "../uni/string.h"

namespace iri { struct IRI; }

namespace ayu {
using namespace std::literals;
using namespace uni;
using iri::IRI;

///// BASIC TYPES AND STUFF

 // Defined elsewhere
struct Document;
struct Dynamic;
struct Location;
using LocationRef = CopyRef<Location>;
struct Pointer;
struct Reference;
struct Resource;
struct Tree;
using TreeRef = CRef<Tree, 16>;
struct Type;

 // Using a Tree as a string-like type.
 // TODO: use uni::AnyString (TODO: implement uni::AnyString)
using TreeString = Tree;

using TreeArray = SharedArray<Tree>;
using TreeArraySlice = Slice<Tree>;
using TreePair = std::pair<TreeString, Tree>;
using TreeObject = SharedArray<TreePair>;
using TreeObjectSlice = Slice<TreePair>;

 // Unknown type that will never be defined.  This has a similar role to void,
 // except:
 //   - You can have a reference Mu& or Mu&&.
 //   - A pointer or reference to Mu is always supposed to refer to a
 //     constructed item, not an unconstructed buffer.  Functions that take or
 //     return unconstructed or untyped buffers use void* instead.
 //   - This does not track constness (in general there shouldn't be any
 //     const Mu&).
struct Mu;

///// UTILITY

void dump_refs (const std::vector<Reference>&);
 // Primarily for debugging.  Prints item_to_string(Reference(&v)) to stderr
template <class... Args>
void dump (const Args&... v) {
    dump_refs({&v...});
}

///// BASIC ERRORS

 // Base class for ayu-related errors.  Do not throw this or a base class
 // directly; throw X<SomethingError>(...) instead.
struct Error {
    const std::source_location* source_location;
};
 // Unclassified error
struct GenericError : Error {
    std::string mess;
};
 // General IO-related problem
struct IOError : Error {
    std::string filename;
    int errnum; // TODO: use std::errc
};
 // Failure to open a file
struct OpenFailed : IOError { };
 // Failure to read from an open file
struct ReadFailed : IOError { };
 // Failure to close a file
struct CloseFailed : IOError { };

} // namespace ayu

