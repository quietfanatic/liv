// An ayu::Pointer is a runtime-typed pointer.  It is trivially copyable and
// destructable, and can be casted from and to native pointers.

#pragma once
#include <cassert>
#include "type.h"

namespace ayu {

struct Pointer {
    Mu* address = null;
    Type type;

    constexpr Pointer () { }
    constexpr Pointer (Mu* a, Type t) : address(a), type(t) { }

    template <class T>
    Pointer (T* a) : address(a), type(Type::CppType<T>()) { }

     // Returns false if this Pointer is either (typed) null or (typeless)
     // empty.
    constexpr explicit operator bool () const { return address; }
     // Returns true only for the typeless empty Pointer.
    constexpr bool empty () const {
        assert(!!address == !!type);
        return !!type;
    }

    template <class T>
    T* try_upcast_to () const { return type.try_upcast_to<T>(address); }
    template <class T>
    T* upcast_to () const { return type.upcast_to<T>(address); }
    template <class T>
    T* try_downcast_to () const { return type.try_downcast_to<T>(address); }
    template <class T>
    T* downcast_to () const { return type.downcast_to<T>(address); }
    template <class T>
    T* try_cast_to () const { return type.try_cast_to<T>(address); }
    template <class T>
    T* cast_to () const { return type.cast_to<T>(address); }

    template <class T>
    operator T* () const { return upcast_to<T>(); }
};

} // namespace ayu
