#pragma once

#include <functional>  // for std::hash
#include <typeinfo>

#include "internal/common-internal.h"
#include "internal/type-internal.h"

namespace ayu {

 // Represents a type known to ayu.  Provides dynamically-typed construction and
 // destruction for any type as long as it has an AYU_DESCRIBE declaration.
 // Can represent const types (called readonly in AYU), but not reference or
 // volatile types.
 //
 // The default value will cause null derefs if you do anything with it.
 //
 // Due to a constellation of unfortunate language rules, Types cannot be
 // constexpr (except the empty Type).  However, they can be constructed at init
 // time, and theoretically with LTO they should be effectively
 // constant-initializable.  You cannot, however, actually use Types for
 // anything before main() starts.
struct Type {
     // Uses a tagged pointer; the first bit determines readonly (const), and the rest
     // points to an ayu::in::Description.
    usize data;

    constexpr Type () : data(0) { }
     // Construct from internal data
    Type (const in::Description* desc, bool readonly = false) :
        data(reinterpret_cast<usize>(desc) | readonly) { }
     // Can throw UnknownType.  There is no way to extract information about
     // constness from a std::type_info, so it must be provided as a bool.
    Type (const std::type_info& t, bool readonly = false) :
        Type(in::get_description_for_type_info(t), readonly)
    { }
     // Should never throw, and in fact compile to a single pointer return.
    template <class T>
        requires (!std::is_volatile_v<std::remove_reference_t<T>>)
    static Type CppType () {
        return Type(
            in::get_description_for_cpp_type<
                std::remove_const_t<std::remove_reference_t<T>>
            >(),
            std::is_const_v<T>
        );
    }
     // Can throw TypeNotFound
    Type (Str name, bool readonly = false) :
        Type(in::need_description_for_name(name), readonly)
    { }

     // Checks if this is the empty type.
    explicit constexpr operator bool () const { return data & ~1; }
     // Checks if this type is readonly (const).
    bool readonly () const { return data & 1; }
     // Add or remove readonly bit
    Type add_readonly () const {
        return Type(reinterpret_cast<const in::Description*>(data & ~1), true);
    }
    Type remove_readonly () const {
        return Type(reinterpret_cast<const in::Description*>(data & ~1), false);
    }

     // Get human-readable type name (whatever name was registered with
     // AYU_DESCRIBE).  This ignores the readonly bit.
    Str name () const;
     // Get the std::type_info& for this type.  NOTE: CONSTNESS INFO IS
     // CURRENTLY NYI
    const std::type_info& cpp_type () const;
     // Get the sizeof() of this type
    usize cpp_size () const;
     // Get the alignof() of this type
    usize cpp_align () const;
     // Construct an instance of this type in-place.  The target must have at
     // least the required size and alignment.
    void default_construct (void* target) const;
     // Destory an instance of this type in-place.  The memory will not be
     // allocated.
    void destroy (Mu*) const;
     // Allocate a buffer appropriate for containing an instance of this type.
     // It is not specified whether this uses new or malloc, so if you use this
     // to allocate space for an object, you must use deallocate() to deallocate
     // it.
    void* allocate () const;
     // Deallocate a buffer previously allocated with allocate()
    void deallocate (void*) const;
     // Allocate and construct an instance of this type.
    Mu* default_new () const;
     // Destruct and deallocate and instance of this type.
     // Should be called delete, but, you know
    void delete_ (Mu*) const;

     // Cast from derived class to base class.  Does a depth-first search
     // through the derived class's description looking for accessors like:
     //  - delegate(...)
     //  - attr("name", ..., inherit)
     //  - elem(..., inherit)
     // and recurses through those accessors.  Note also only information
     // provided through AYU_DESCRIBE will be used; C++'s native inheritance
     // system has no influence.
     //
     // try_upcast_to will return null if the requested base class was not found
     // in the derived class's inheritance hierarchy, or if the address of the
     // base class can't be retrieved (goes through value_funcs or some such).
     // upcast_to will throw CannotCoerce (unless given null, in which case
     // it will return null).
     //
     // Finally, casting from non-readonly to readonly types is allowed, but not
     // vice versa.
    Mu* try_upcast_to (Type, Mu*) const;
    template <class T>
    T* try_upcast_to (Mu* p) const {
        return (T*)try_upcast_to(Type::CppType<T>(), p);
    }
    Mu* upcast_to (Type, Mu*) const;
    template <class T>
    T* upcast_to (Mu* p) const {
        return (T*)upcast_to(Type::CppType<T>(), p);
    }

     // Cast from base class to derived class.  See upcast_to for more details.
     //
     // One difference from upcast_to is that while upcast_to can follow any
     // accessors with the "address" operation, downcast_to can only follow
     // accessors with the "inverse_address" operation, namely base<>() and
     // member().
     //
     // As with C++'s static_cast, this cannot check that the pointed-to data
     // really is the derived class, and if it isn't, incorrect execution
     // may occur.
     //
     // Unlike upcast, downcast may cast from readonly to non-readonly.  As with
     // C++'s const_cast, modifying the pointed-to data may cause undefined
     // behavior.
    Mu* try_downcast_to (Type, Mu*) const;
    template <class T>
    T* try_downcast_to (Mu* p) const {
        return (T*)try_downcast_to(Type::CppType<T>(), p);
    }
    Mu* downcast_to (Type, Mu*) const;
    template <class T>
    T* downcast_to (Mu* p) const {
        return (T*)downcast_to(Type::CppType<T>(), p);
    }

     // Try upcast, then downcast.
    Mu* try_cast_to (Type, Mu*) const;
    template <class T>
    T* try_cast_to (Mu* p) const {
        return (T*)try_cast_to(Type::CppType<T>(), p);
    }
    Mu* cast_to (Type, Mu*) const;
    template <class T>
    T* cast_to (Mu* p) const {
        return (T*)cast_to(Type::CppType<T>(), p);
    }
};

 // The same type will always have the same description pointer.
inline bool operator == (Type a, Type b) {
    return a.data == b.data;
}
inline bool operator != (Type a, Type b) {
    return a.data != b.data;
}

struct TypeError : Error { };
 // Tried to map a C++ type to an AYU type, but AYU doesn't know about this
 // type (it has no AYU_DESCRIBE description).
 // TODO: serializing this doesn't work?
struct UnknownType : TypeError {
    const std::type_info& cpp_type;
};
 // Tried to look up a type by name, but there is no type with that name.
struct TypeNotFound : TypeError {
    String name;
};
 // Tried to default construct a type that has no default constructor.
struct CannotDefaultConstruct : TypeError {
    Type type;
};
 // Tried to construct or destroy a type that has no destructor.
struct CannotDestroy : TypeError {
    Type type;
};
 // Tried to coerce between types that can't be coerced.
struct CannotCoerce : TypeError {
    Type from;
    Type to;
};

} // namespace ayu

 // Allow hashing Type for std::unordered_map
template <>
struct std::hash<ayu::Type> {
    size_t operator () (ayu::Type t) const {
        return hash<void*>()((void*)t.data);
    }
};

