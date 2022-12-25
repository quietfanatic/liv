// CopyRef<> / CRef<>
// Pass-by-const-reference semantics with pass-by-value performance.
//
// CopyRef<T> is a class that acts like a const reference to an object of type
// T, but it's representation is actually a bit copy of the object.
//
// Alternatively, CopyRef<T> can be though of as a way to copy objects while
// dodging their copy constructors and destructors, e.g. for an object that
// counts references this skips the reference increment/decrement.
//
// CopyRef<T> is only recommended for objects small enough to be passed by value
// in the current ABI.  For most ABIs this is the size of two pointers (it's one
// pointer on Microsoft x64).
//
// You can use CopyRef<T> if all of the following are true, but most of these
// requirements are not enforcible in current C++.
//   - T is movable and not trivially copiable.
//   - T has no members marked as mutable.
//   - T has no behavior that depends on its address staying constant.
//   - T will not be modified by other code while you have a reference to it.
//
// ConstRef<T> is a thin wrapper around const T&, to be used with CRef<T>.
//
// CRef<T> selects between CopyRef<T> or ConstRef<T> depending on the size of T
// and the current ABI.  If T has not been defined yet, its size cannot be
// determined, so you have to pass the size as a second template parameter e.g.
// CRef<T, 16>.
//
// Like all reference-like types, undefined behavior will result if you keep a
// CopyRef<T> or CRef<T> around longer than the lifetime of the object it
// references.  Unlike const T&, undefined behavior also results if other code
// modifies the original while you have a reference.
//
// Note: The reason CopyRef<T> requires move constructibility even though it
// doesn't do any moves is because if a type is not move-constructible, that
// usually implies that its address is semantically important for it, and the
// CopyRef would have a different address from the original.  The requirement of
// non-trivial copyability isn't really a requirement, but if the object is
// trivially copyable then there's no reason to use CopyRef, so just pass by
// value.

#pragma once

#include <cstring>
#include <utility>
#include "common.h"

namespace uni {

template <class T>
struct CopyRef {
     // Default constructible, but if you reference this before assigning to it,
     // the behavior will be very undefined.
    CopyRef () = default;
     // Make trivially copyable
    CopyRef (const CopyRef<T>&) = default;
     // Implicit coercion from const T&
     // Sadly there is no way to make this constexpr (std::bit_cast only works
     // for types that are already trivially copyable).
    ALWAYS_INLINE CopyRef (const T& t) :
        CopyRef(reinterpret_cast<const CopyRef<T>&>(t))
    { }
     // Implicit coercion to const T&
    ALWAYS_INLINE operator const T& () const {
        return reinterpret_cast<const T&>(*this);
    }
     // Wasn't sure whether to overload this or not, but if the class implements
     // this, you probably want to use whatever *T returns instead of
     // CopyRef<T>.
    ALWAYS_INLINE const T& operator* () const {
        return reinterpret_cast<const T&>(*this);
    }
     // Sadly we can't overload ., so here's the next best alternative
    ALWAYS_INLINE const T* operator-> () const {
        return reinterpret_cast<const T*>(this);
    }
     // Allow assigning
    ALWAYS_INLINE CopyRef<T>& operator= (const CopyRef<T>& o) {
         // Can't default this because you can't assign arrays.
        std::memcpy(this, &o, sizeof(T));
        return *this;
    }
     // Forbid assigning from a T, as that's probably a mistake (T's copy
     // constructor and copy assigner will not be called).  Explictly cast to
     // CopyRef<T> first.
    CopyRef<T>& operator= (const T&) = delete;
     // Because C++ doesn't have Perl's ref->[i] and ref->(foo) syntax, and
     // nobody wants to write (*ref)[i]
    template <class Ix>
    ALWAYS_INLINE CE auto operator [] (Ix i) const {
        return reinterpret_cast<const T&>(*this)[i];
    }
    template <class... Args>
    ALWAYS_INLINE CE auto operator () (Args&&... args) const {
        return reinterpret_cast<const T&>(*this)(std::forward<Args>(args)...);
    }
  private:
    alignas(T) const char repr [sizeof(T)];
};

 // For use with CRef to be source-compatible with CopyRef.  If you're going to
 // use this directly, just use const T& instead.
template <class T>
struct ConstRef {
    ALWAYS_INLINE ConstRef (const ConstRef<T>&) = default;
    ALWAYS_INLINE ConstRef (const T& ref) : ref(ref) { }
    ALWAYS_INLINE operator const T& () const { return ref; }
    ALWAYS_INLINE const T& operator* () const { return ref; }
    ALWAYS_INLINE const T* operator-> () const { return &ref; }
    ConstRef<T>& operator= (const ConstRef<T>&) = delete;
  private:
    const T& ref;
};

 // Most ABIs support pass-by-value of up to twice the size of a register.  The
 // most major exception is Microsoft x64.
#ifndef CONFIG_PASS_BY_VALUE_MAX_SIZE
#if _MSC_VER && _M_AMD64
#define CONFIG_PASS_BY_VALUE_MAX_SIZE 8
#else
#define CONFIG_PASS_BY_VALUE_MAX_SIZE (2*sizeof(void*))
#endif
#endif

template <class T, usize size = sizeof(T)>
using CRef = std::conditional_t<
    size <= CONFIG_PASS_BY_VALUE_MAX_SIZE,
    CopyRef<T>, ConstRef<T>
>;

} // namespace uni
