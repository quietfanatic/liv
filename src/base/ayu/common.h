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
#include "../uni/callback.h"
#include "../uni/common.h"

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
struct Pointer;
struct Reference;
struct Resource;
struct Tree;
struct Type;

 // Using a Tree as a string-like type.
using TreeString = Tree;

 // String is for storage and function return.  Str is for function parameters.
 // (you can also take String&& as a parameter if you're going to store it)
using String = std::string;
using Str = std::string_view;
 // Ayu works natively with UTF-8, but can convert to and from UTF-16.
using String16 = std::u16string;
using Str16 = std::u16string_view;

using Array = std::vector<Tree>;
using Pair = std::pair<TreeString, Tree>;
using Object = std::vector<Pair>;

 // Unknown type that will never be defined.  This has a similar role to void,
 // except:
 //   - You can have a reference Mu& or Mu&&.
 //   - A pointer or reference to Mu is always supposed to refer to a
 //     constructed item, not an unconstructed buffer.  Functions that take or
 //     return unconstructed or untyped buffers use void* instead.
 //   - This does not track constness (in general there shouldn't be any
 //     const Mu&).
struct Mu;

///// STRINGS

namespace in {
    template <class T>
    static auto to_string (T&& s) {
        if constexpr (
            !std::is_same_v<std::decay_t<T>, char>
            && requires (T v) { std::to_string(v); }
        ) return std::to_string(std::forward<T>(s));
        else return std::forward<T>(s);
    }
}

 // I'm sick and tired of weirdness around string concatenation operators.
 // Just use this.  It will probably end up being more efficient anyway.
 // TODO: Simplify the overloads
template <class... Args>
String cat (Args&&... args) {
    String r; // Should we reserve()?  Profile!
    ((r += in::to_string(std::forward<Args>(args))), ...);
    return r;
}
 // Optimization to skip a copy
template <class... Args>
String cat (String&& s, Args... args) {
    String r = std::move(s);
    ((r += in::to_string(std::forward<Args>(args))), ...);
    return r;
}

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
    String mess;
};
 // General IO-related problem
struct IOError : Error {
    String filename;
    int errnum; // TODO: use std::errc
};
 // Failure to open a file
struct OpenFailed : IOError { };
 // Failure to read from an open file
struct ReadFailed : IOError { };
 // Failure to close a file
struct CloseFailed : IOError { };

} // namespace ayu

