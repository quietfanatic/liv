// CopyRef<> / CRef<>
// Pass-by-const-reference semantics with pass-by-value performance.
//
// Use CopyRef<T> if all of the following are true about T:
//   - is small (8 bytes on most ABIs, 16 on sys-v 64)
//   - has a non-trivial move constructor
//   - has no members marked as mutable
//   - has no behavior that depends on its address staying constant
//   - will not be modified by other code while you have a reference to it.
//
// Alternatively, CopyRef<T> can be though of as a way to copy objects while
// dodging their copy constructors and destructors, e.g. for an object that
// counts references this skips the reference increment/decrement.
//
// CRef<T> selects between CopyRef<T> and const T& depending on its size and the
// current ABI.  If T has not been defined yet, its size cannot be determined,
// so you have to pass the size as a second template parameter e.g. CRef<T, 16>
//
// Like all reference-like types, undefined behavior will result if you keep a
// CopyRef<T> or CRef<T> around longer than the lifetime of the object it
// references.

#pragma once

#include <cstring>
#include <type_traits>

namespace uni {

template <class T>
struct CopyRef {
     // Default constructible, but if you reference this before assigning to it,
     // the behavior will be very undefined.
    [[gnu::always_inline]]
    CopyRef () = default;
     // Make trivially copyable
    [[gnu::always_inline]]
    CopyRef (const CopyRef<T>&) = default;
     // Implicit coercion from const T&
     // Sadly there is no way to make this constexpr (std::bit_cast only works
     // for types that are already trivially copyable).
    [[gnu::always_inline]]
    CopyRef (const T& t) :
        CopyRef(reinterpret_cast<const CopyRef<T>&>(t))
    { }
     // Implicit coercion to const T&
    [[gnu::always_inline]]
    operator const T& () const {
        return reinterpret_cast<const T&>(*this);
    }
     // Wasn't sure whether to overload this or not, but if the class implements
     // this, you probably want to use whatever *T returns instead of
     // CopyRef<T>.
    [[gnu::always_inline]]
    const T& operator* () const {
        return reinterpret_cast<const T&>(*this);
    }
     // Sadly we can't overload ., so here's the next best alternative
    [[gnu::always_inline]]
    const T* operator-> () const {
        return reinterpret_cast<const T*>(storage);
    }
     // Allow assigning
    [[gnu::always_inline]]
    CopyRef<T>& operator= (const CopyRef<T>& o) {
         // Can't default this because you can't assign arrays.
        std::memcpy(this, &o, sizeof(T));
        return *this;
    }
     // Forbid assigning from a T, as that's probably a mistake (T's copy
     // constructor and copy assigner will not be called).  Explictly cast to
     // CopyRef<T> first.
    CopyRef<T>& operator= (const T&) = delete;
  private:
    alignas(T) const char storage [sizeof(T)];
};

 // For use with CRef to be source-compatible with CopyRef.  If you're going to
 // use this directly, just use const T& instead.
template <class T>
struct ConstRef {
    [[gnu::always_inline]]
    ConstRef (const ConstRef<T>&) = default;
    [[gnu::always_inline]]
    ConstRef (const T& ref) : ref(ref) { }
    [[gnu::always_inline]]
    operator const T& () const { return ref; }
    [[gnu::always_inline]]
    const T& operator* () const { return ref; }
    [[gnu::always_inline]]
    const T* operator-> () const { return &ref; }
    ConstRef<T>& operator= (const ConstRef<T>&) = delete;
  private:
    const T& ref;
};

#ifdef _MSC_VER
template <class T, usize size = sizeof(T)>
using CRef = std::conditional_t<size <= 8, CopyRef<T>, const T&>;
#else
template <class T, usize size = sizeof(T)>
using CRef = std::conditional_t<size <= 2*sizeof(void*), CopyRef<T>, const T&>;
#endif

} // namespace uni
