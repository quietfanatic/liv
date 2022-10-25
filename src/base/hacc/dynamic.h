#pragma once

#include <type_traits>

#include "common.h"
#include "type.h"

namespace hacc {

 // Represents a dynamically typed object with value semantics.  This is
 //  always allocated on the heap.  Can only represent haccable types.
 // The empty value will cause null derefs if you do anything with it.
struct Dynamic {
    const Type type;
    Mu* const data;

    constexpr Dynamic () : type(), data(null) { }
    constexpr Dynamic (Type t, Mu* d) : type(t), data(d) { }

     // Default construction
    explicit Dynamic (Type t) :
        type(t),
        data(t ? t.default_new() : null)
    { }

    Dynamic (Dynamic&& o) : type(o.type), data(o.data) {
        const_cast<Type&>(o.type) = Type();
        const_cast<Mu*&>(o.data) = null;
    }
    template <class T, in::disable_if_Dynamic_or_Type<T> = true>
    Dynamic (T&& v) :
        Dynamic(Type::CppType<T>(), reinterpret_cast<Mu*>(new T (std::move(v))))
    { }

    template <class T, class... Args>
    static Dynamic make (Args&&... args) {
        return Dynamic(Type::CppType<T>(), new T {std::forward<Args...>(args...)});
    }

    Dynamic& operator = (Dynamic&& o) {
        this->~Dynamic();
        new (this) Dynamic (std::move(o));
        return *this;
    }

    ~Dynamic () {
        if (data) type.delete_(data);
    }

    bool has_value () const { return !!type; }

    Mu& as (Type t) {
        return *type.cast_to(t, data);
    }
    const Mu& as (Type t) const {
        return *type.cast_to(t, (const Mu*)data);
    }

    template <class T>
    in::remove_cvref<T>& as () {
        return reinterpret_cast<in::remove_cvref<T>&>(
            as(Type::CppType<in::remove_cvref<T>>())
        );
    }
    template <class T>
    const in::remove_cvref<T>& as () const {
        return reinterpret_cast<const in::remove_cvref<T>&>(
            as(Type::CppType<in::remove_cvref<T>>())
        );
    }
    template <class T>
    in::remove_cvref<T> get () const {
        return as<in::remove_cvref<T>>();
    }

    template <class T, in::disable_if_Dynamic_or_Type<T> = true>
    explicit operator T& () {
        return as<T>();
    }
    template <class T, in::disable_if_Dynamic_or_Type<T> = true>
    explicit operator const T& () const {
        return as<T>();
    }
};

} // namespace hacc
