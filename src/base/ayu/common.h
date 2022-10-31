// This module contains various types and exceptions that are used throughout
// the library.

#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
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
static String operator + (Str a, const String& b) { return String(a) + b; }
static String operator + (const String& a, Str b) { return a + String(b); }

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

void dump_ref (const Reference&);
 // Primarily for debugging.  Prints item_to_string(Reference(&v)) to stderr
template <class T>
void dump (const T& v) { dump_ref(&v); }

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
        int errnum;
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

///// INTERNAL STUFF THAT HAS TO BE IN THE HEADER ANYWAY
// TODO: Put this in internal folder

namespace in {
    struct DocumentData;
    struct Description;
    struct PathData;
    void delete_PathData (PathData*);
    struct ResourceData;
    struct UniverseData;
    struct TreeData;
    void delete_TreeData (TreeData*);

     // Intrusive reference counting
    struct RefCounted {
        mutable uint32 ref_count = 0;
    };
     // T must be BINARY-COMPATIBLE with RefCounted.
     // This means RefCounted must be the FIRST BASE and NO VIRTUAL METHODS.
     // I haven't thought of a way to enforce this yet.
     // The benefit to this is that T need not be complete to use this class.
    template <class T, void(& deleter )(T*)>
    struct RCP {
        T* p;

        void inc () {
            if (p) {
                reinterpret_cast<RefCounted*>(p)->ref_count++;
            }
        }
        void dec () {
            if (p && !--reinterpret_cast<RefCounted*>(p)->ref_count) {
                deleter(p);
            }
        }

        constexpr RCP (Null) : p(null) { }
        RCP (T* p) : p(p) { inc(); }
        RCP (const RCP& o) : p(o.p) { inc(); }
        RCP (RCP&& o) : p(o.p) { o.p = null; }
        ~RCP () { dec(); }

        RCP& operator = (const RCP& o) { dec(); p = o.p; inc(); return *this; }
        RCP& operator = (RCP&& o) { dec(); p = o.p; o.p = null; return *this; }

         // It's up to the owning class to maintain const-correctness.
        T& operator * () const { return *p; }
        T* operator -> () const { return p; }
        explicit operator bool () const { return p; }
    };
    template <class T, void(& deleter )(T*)>
    bool operator == (const RCP<T, deleter>& a, const RCP<T, deleter>& b) {
        return a.p == b.p;
    }
    template <class T, void(& deleter )(T*)>
    bool operator != (const RCP<T, deleter>& a, const RCP<T, deleter>& b) {
        return a.p != b.p;
    }

     // Some type traits and stuff
    template <class T>
    using disable_if_Dynamic_or_Type = std::enable_if_t<
        !std::is_base_of_v<Dynamic, std::decay_t<T>>
        && !std::is_base_of_v<Type, std::decay_t<T>>
    , bool>;
     // Don't have this until C++20
    template <class T>
    using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

    static usize hash_combine (usize a, usize b) {
        return a*3 + b;
    }

     // Called when an exception is thrown in a place where the library can't
     //  properly clean up after itself, such as when a resource value throws
     //  from its destructor.
    [[noreturn]] void unrecoverable_exception (std::exception& e, Str when);
     // Some internal error has occured, such as an invalid enum value, and i
     //  isn't safe to continue execution.
    [[noreturn]] void internal_error (
        const char* function, const char* filename, uint line
    );
#define AYU_INTERNAL_ERROR() \
    ::ayu::in::internal_error(__FUNCTION__, __FILE__, __LINE__);
}

}  // namespace ayu
