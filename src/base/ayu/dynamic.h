// Represents a dynamically typed object with value semantics.  This is always
// allocated on the heap.  Can only represent types known to ayu.  Can be moved
// but not copied.  There is an empty Dynamic which has no type and no value,
// but unlike Reference, there is no "null" Dynamic which has type and no value.
// If there is a type there is a value, and vice versa.
#pragma once

#include <cassert>
#include <type_traits>

#include "internal/common-internal.h"
#include "type.h"

namespace ayu {

 // TODO: Should we rename this to Any?
struct Dynamic {
    const Type type;
    Mu* const data;

     // The empty value will cause null derefs if you do anything with it.
    constexpr Dynamic () : type(), data(null) { }
     // Create from internal data.  Takes ownership.
    Dynamic (Type t, Mu*&& d) : type(t), data(d) { d = null; }
     // Default construction
    explicit Dynamic (Type t) :
        type(t),
        data(t ? t.default_new() : null)
    { }
     // Move construct
    Dynamic (Dynamic&& o) : type(o.type), data(o.data) {
        const_cast<Type&>(o.type) = Type();
        const_cast<Mu*&>(o.data) = null;
    }
     // Construct by moving an arbitrary type in
    template <class T> requires (
        !std::is_base_of_v<Dynamic, T>
        && !std::is_base_of_v<Type, T>
        && !std::is_reference_v<T>
    )
    Dynamic (T&& v) : type(Type::CppType<T>()), data(reinterpret_cast<Mu*>(type.allocate())) {
        try {
            new (data) T (std::move(v));
        }
        catch (...) {
            type.deallocate(data);
            throw;
        }
    }
     // Construct with arguments.
    template <class T, class... Args>
    static Dynamic make (Args&&... args) {
        auto type = Type::CppType<T>();
        void* buf = type.allocate();
        try {
            return Dynamic(
                Type::CppType<T>(),
                new (buf) T {std::forward<Args...>(args...)}
            );
        }
        catch (...) {
            type.deallocate(buf);
            throw;
        }
    }
     // Move assignment
    Dynamic& operator = (Dynamic&& o) {
        this->~Dynamic();
        new (this) Dynamic (std::move(o));
        return *this;
    }
     // Destroy
    ~Dynamic () {
        if (data) type.delete_(data);
    }
     // Check contents.  No coercion to bool because that would be confusing.
    bool has_value () const {
        assert(!!type == !!data);
        return !!type;
    }
    bool empty () const {
        assert(!!type == !!data);
        return !type;
    }
     // Runtime casting
    Mu& as (Type t) {
        return *type.cast_to(t, data);
    }
    const Mu& as (Type t) const {
        return *type.cast_to(t, (const Mu*)data);
    }
    template <class T>
    std::remove_cvref_t<T>& as () {
        return reinterpret_cast<std::remove_cvref_t<T>&>(
            as(Type::CppType<std::remove_cvref_t<T>>())
        );
    }
    template <class T>
    const std::remove_cvref_t<T>& as () const {
        return reinterpret_cast<const std::remove_cvref_t<T>&>(
            as(Type::CppType<std::remove_cvref_t<T>>())
        );
    }
     // Copying accessor
    template <class T>
    std::remove_cvref_t<T> get () const {
        return as<std::remove_cvref_t<T>>();
    }
     // Explicit coercion
    template <class T> requires (
        !std::is_base_of_v<Dynamic, T>
        && !std::is_base_of_v<Type, T>
        && !std::is_reference_v<T>
    )
    explicit operator T& () {
        return as<T>();
    }
    template <class T> requires (
        !std::is_base_of_v<Dynamic, T>
        && !std::is_base_of_v<Type, T>
        && !std::is_reference_v<T>
    )
    explicit operator const T& () const {
        return as<T>();
    }
};

} // namespace ayu
