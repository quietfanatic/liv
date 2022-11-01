// This module contains various types and exceptions that are used throughout
// the library.

#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ayu {

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
struct Path;
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
 // Dunno why the standard library doesn't have this
inline String operator + (Str a, const String& b) { return String(a) + b; }
inline String operator + (const String& a, Str b) { return a + String(b); }

using Array = std::vector<Tree>;
using Pair = std::pair<String, Tree>;
using Object = std::vector<Pair>;

 // A super lightweight callback class with reference semantics (std::function
 // has value semantics and can be copied and moved, so it's way bigger.)
 // TODO: We should probably be able to make this movable though.
template <class> struct CallbackV;
template <class Ret, class... Args>
struct CallbackV<Ret(Args...)> {
    Ret(* wrapper )(const void*, Args&&...);
    const void* f;
    template <class F, std::enable_if_t<
        std::is_convertible_v<std::invoke_result_t<F, Args...>, Ret>, bool
    > = true>
    constexpr CallbackV (const F& f) :
        wrapper([](const void* f, Args&&... args)->Ret{
            return (*reinterpret_cast<const F*>(f))(std::forward<Args>(args)...);
        }),
        f(&f)
    { }
     // Looks like there's no way to avoid an extra copy of by-value args.
     // (std::function does it too)
    Ret operator () (Args... args) const {
        return wrapper(f, std::forward<Args>(args)...);
    }
};
template <class Sig>
using Callback = const CallbackV<Sig>&;

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
     // Things like incorrect attribute names, type mismatches
    struct LogicError : Error { };
     // Unclassified error
    struct GenericError : LogicError {
        String mess;
        GenericError (String&& m) : mess(m) { }
    };
     // Errors that only occur in debug builds
    struct DebugError : Error { };
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
     // Failure to close a file
    struct CloseFailed : IOError {
        using IOError::IOError;
    };

}

} // namespace ayu

