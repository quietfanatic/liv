#pragma once

#include "../common.h"

#include <type_traits>

namespace ayu::in {

 // Predeclare some private classes
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
 // properly clean up after itself, such as when a resource value throws
 // from its destructor.
[[noreturn]] void unrecoverable_exception (std::exception& e, Str when);
 // Some internal error has occured, such as an invalid enum value, and it
 // isn't safe to continue execution.
[[noreturn]] void internal_error (
    const char* function, const char* filename, uint line
);
#define AYU_INTERNAL_UGUU() \
::ayu::in::internal_error(__FUNCTION__, __FILE__, __LINE__);

}  // namespace ayu::in
