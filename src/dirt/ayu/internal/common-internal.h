#pragma once

#include "../common.h"

#include <type_traits>

namespace ayu::in {

 // Predeclare some private classes
struct DocumentData;
struct Description;
struct LocationData;
void delete_LocationData (LocationData*);
struct ResourceData;

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

    constexpr void inc () {
        if (p) {
            reinterpret_cast<RefCounted*>(p)->ref_count++;
        }
    }
    constexpr void dec () {
        if (p && !--reinterpret_cast<RefCounted*>(p)->ref_count) {
            deleter(p);
        }
    }

    constexpr RCP (Null n = null) : p(n) { }
    constexpr RCP (T* p) : p(p) { inc(); }
    constexpr RCP (const RCP& o) : p(o.p) { inc(); }
    RCP (RCP&& o) : p(o.p) { o.p = null; }
    constexpr ~RCP () { dec(); }

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

inline usize hash_combine (usize a, usize b) {
    return a*3 + b;
}

}  // namespace ayu::in
