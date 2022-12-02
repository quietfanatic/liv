// This module contains various types and exceptions that are used throughout
// the library.

#pragma once

#include <cstdint>
#include <cwchar>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace iri { struct IRI; }

namespace ayu {

using namespace std::literals;

///// BASIC TYPES AND STUFF

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using isize = std::intptr_t;
using uint = unsigned int;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using usize = std::uintptr_t;

using Null = std::nullptr_t;
constexpr Null null = nullptr;

constexpr double nan = std::numeric_limits<double>::quiet_NaN();
constexpr double inf = std::numeric_limits<double>::infinity();

 // Unknown type that will never be defined
struct Mu;

 // Defined elsewhere
struct Document;
struct Dynamic;
struct Location;
struct Reference;
struct Resource;
struct Tree;
struct Type;

 // String is for storage and function return.  Str is for function parameters.
 // (you can also take String&& as a parameter if you're going to store it)
using String = std::string;
using Str = std::string_view;
 // Ayu works natively with UTF-8, but can convert to and from UTF-16.
using String16 = std::u16string;
using Str16 = std::u16string_view;

using Array = std::vector<Tree>;
using Pair = std::pair<String, Tree>;
using Object = std::vector<Pair>;

using iri::IRI;

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
template <class... Args>
String cat (Args&&... args) {
    String r; // Should we reserve()?  Profile!
    ((r += in::to_string(std::forward<Args>(args))), ...);
    return r;
}
 // Optimization to skip a copy
template <class... Args>
String&& cat (String&& s, Args... args) {
    ((s += in::to_string(std::forward<Args>(args))), ...);
    return std::move(s);
}

///// CALLBACKS

 // A super lightweight callback class with reference semantics (std::function
 // has value semantics and can be copied and moved, so it's way bigger.)
 // The most we can reduce this to is two function calls (four uninlined), but
 // with a gnu extension allowing you to coerce a method pointer into a function
 // pointer, we could theoretically reduce this to one call (TODO: do that).
 // I mean, using a method pointer itself would be one call but C++ method
 // pointers are kind of bad.
 // THEORETICALLY theoretically, if we knew details on how the compiler
 // implements method pointers we could do it for any compiler.  No.
template <class> struct CallbackV;
template <class Ret, class... Args>
struct CallbackV<Ret(Args...)> {
    Ret(* wrapper )(const void*, Args&&...);
    const void* f;
    template <class F> requires(
        std::is_convertible_v<std::invoke_result_t<F, Args...>, Ret>
    )
    [[gnu::always_inline]]
    constexpr CallbackV (const F& f) :
        wrapper([](const void* f, Args&&... args)->Ret{
            return (*reinterpret_cast<const F*>(f))(std::forward<Args>(args)...);
        }),
        f(&f)
    { }
     // Looks like there's no way to avoid an extra copy of by-value args.
     // (std::function does it too)
    [[gnu::always_inline]]
    Ret operator () (Args... args) const {
        return wrapper(f, std::forward<Args>(args)...);
    }
};
template <class Sig>
using Callback = const CallbackV<Sig>&;

///// UTILITY

void dump_refs (const std::vector<Reference>&);
 // Primarily for debugging.  Prints item_to_string(Reference(&v)) to stderr
template <class... Args>
void dump (const Args&... v) {
    dump_refs({&v...});
}

///// BASIC ERRORS

namespace X {
     // Base class for ayu-related errors.
    struct Error : std::exception {
         // Gotta cache the generated error message or the exception handling
         // system will reference stack garbage.
        mutable String mess_cache;
         // Calls item_to_string on the most derived type
        const char* what () const noexcept override;
    };
     // Unclassified error
    struct GenericError : Error {
        String mess;
        GenericError (String&& m) : mess(m) { }
    };
     // General IO-related problem
    struct IOError : Error {
        String filename;
        int errnum; // TODO: use std::errc
        IOError (Str f, int e) : filename(f), errnum(e) { }
    };
     // Failure to open a file
    struct OpenFailed : IOError {
        using IOError::IOError;
    };
     // Failure to read from an open file
    struct ReadFailed : IOError {
        using IOError::IOError;
    };
     // Failure to close a file
    struct CloseFailed : IOError {
        using IOError::IOError;
    };
}

} // namespace ayu

