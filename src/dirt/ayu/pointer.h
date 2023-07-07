// An ayu::Pointer is a runtime-typed pointer.  It is trivially copyable and
// destructable, and can be casted from and to native pointers.
//
// Pointers cannot be constructed until main() starts (except for the typeless
// empty Pointer).

#pragma once
#include <cassert>
#include "type.h"

namespace ayu {

struct Pointer {
    Mu* address;
    Type type;

    constexpr Pointer (Null n = null) : address(n) { }
    Pointer (Type t, Mu* a) : address(a), type(t) { }

    template <class T>
        requires (!std::is_same_v<std::remove_cv_t<T>, void>
               && !std::is_same_v<std::remove_cv_t<T>, Mu>)
    Pointer (T* a) : address((Mu*)a), type(Type::CppType<T>()) { }

     // Returns false if this Pointer is either (typed) null or (typeless)
     // empty.
    constexpr explicit operator bool () const { return address; }
     // Returns true only for the typeless empty Pointer.
    constexpr bool empty () const {
        assert(!!address == !!type);
        return !!type;
    }

    Pointer try_upcast_to (Type t) const {
        return Pointer(t, type.try_upcast_to(t, address));
    }
    template <class T>
    T* try_upcast_to () const {
        return type.try_upcast_to<T>(address);
    }

    Pointer upcast_to (Type t) const {
        return Pointer(t, type.upcast_to(t, address));
    }
    template <class T>
    T* upcast_to () const {
        return type.upcast_to<T>(address);
    }

    Pointer try_downcast_to (Type t) const {
        return Pointer(t, type.try_downcast_to(t, address));
    }
    template <class T>
    T* try_downcast_to () const {
        return type.try_downcast_to<T>(address);
    }

    Pointer downcast_to (Type t) const {
        return Pointer(t, type.downcast_to(t, address));
    }
    template <class T>
    T* downcast_to () const {
        return type.downcast_to<T>(address);
    }

    Pointer try_cast_to (Type t) const {
        return Pointer(t, type.try_cast_to(t, address));
    }
    template <class T>
    T* try_cast_to () const {
        return type.try_cast_to<T>(address);
    }

    Pointer cast_to (Type t) const {
        return Pointer(t, type.cast_to(t, address));
    }
    template <class T>
    T* cast_to () const {
        return type.cast_to<T>(address);
    }

    template <class T>
        requires (!std::is_same_v<std::remove_cv_t<T>, void>
               && !std::is_same_v<std::remove_cv_t<T>, Mu>)
    operator T* () const { return type.upcast_to<T>(address); }
};

constexpr bool operator == (const Pointer& a, const Pointer& b) {
    return a.address == b.address && a.type == b.type;
}
constexpr bool operator != (const Pointer& a, const Pointer& b) {
    return !(a == b);
}

} // namespace ayu

template <>
struct std::hash<ayu::Pointer> {
    std::size_t operator () (const ayu::Pointer& p) const {
        return ayu::in::hash_combine(
            std::hash<ayu::Mu*>{}(p.address),
            std::hash<ayu::Type>{}(p.type)
        );
    }
};
