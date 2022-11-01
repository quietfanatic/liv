#pragma once

#include <functional>  // for std::hash
#include <typeinfo>

#include "internal/common-internal.h"
#include "internal/type-internal.h"

namespace ayu {

 // Represents a type known to ayu.  Provides dynamically-typed construction and
 // destruction for any type as long as it has an AYU_DESCRIBE declaration.
 //
 // The default value will cause null derefs if you do anything with it.
struct Type {
    const in::Description* desc;

    constexpr Type (const in::Description* desc = null) : desc(desc) { }
     // Can throw X::Unayuable
    Type (const std::type_info& t) :
        desc(in::need_description_for_type_info(t))
    { }
    template <class T>
    static Type CppType () {
        return in::need_description_for_cpp_type<T>();
    }
     // Can throw X::TypeNotFound
    Type (Str name) :
        desc(in::need_description_for_name(name))
    { }

    explicit constexpr operator bool () const { return desc; }
    Str name () const;
    const std::type_info& cpp_type () const;
    usize cpp_size () const;
    void default_construct (void* target) const;
    void destruct (Mu&) const;
     // It is not specified whether this uses new or malloc, so if you use this
     // to allocate space for an object, you must use deallocate() to deallocate
     // it.
    void* allocate () const;
    void deallocate (void*) const;
    Mu* default_new () const;
    Mu* copy_new (const Mu&) const;
    Mu* move_new (Mu&&) const;
     // Should be called delete, but, you know
    void delete_ (Mu*) const;

     // For now, all these do is throw an exception if the type doesn't match.
     // Eventually we will implement base conversion.

     // Cast from derived class to base class.  Will throw X::CannotCoerce if
     // the requested Type is not a base of this Type.
    Mu* upcast_to (Type, Mu*) const;
    const Mu* upcast_to (Type t, const Mu* p) const {
        return (const Mu*)upcast_to(t, (Mu*)p);
    }
    template <class T>
    T* upcast_to (Mu* p) const {
        return (T*)upcast_to(Type::CppType<T>(), p);
    }
    template <class T>
    const T* upcast_to (const Mu* p) const {
        return (const T*)upcast_to(Type::CppType<T>(), (Mu*)p);
    }

     // Cast from base class to derived class.  Will throw X::CannotCoerce if
     // the requested Type is not a base of this Type.  As with static_cast,
     // this cannot check that the pointed-to data really is the derived class,
     // and if it isn't, incorrect execution may occur.
    Mu* downcast_to (Type, Mu*) const;
    const Mu* downcast_to (Type t, const Mu* p) const {
        return (const Mu*)downcast_to(t, (Mu*)p);
    }
    template <class T>
    T* downcast_to (Mu* p) const {
        return (T*)downcast_to(Type::CppType<T>(), p);
    }
    template <class T>
    const T* downcast_to (const Mu* p) const {
        return (const T*)downcast_to(Type::CppType<T>(), (Mu*)p);
    }

     // Try upcast, then downcast.
    Mu* cast_to (Type, Mu*) const;
    const Mu* cast_to (Type t, const Mu* p) const {
        return (const Mu*)cast_to(t, (Mu*)p);
    }
    template <class T>
    T* cast_to (Mu* p) const {
        return (T*)cast_to(Type::CppType<T>(), p);
    }
    template <class T>
    const T* cast_to (const Mu* p) const {
        return (const T*)cast_to(Type::CppType<T>(), (Mu*)p);
    }
};

 // The same type will always have the same description pointer.
inline bool operator == (Type a, Type b) {
    return a.desc == b.desc;
}
inline bool operator != (Type a, Type b) {
    return a.desc != b.desc;
}

namespace X {
    struct TypeError : LogicError { };
     // Tried to map a C++ type to an AYU type, but AYU doesn't know about this
     // type (it has no AYU_DESCRIBE description).
     // TODO: serializing this doesn't work?
    struct UnknownType : TypeError {
        const std::type_info& cpp_type;
        UnknownType (const std::type_info& t) : cpp_type(t) { }
    };
     // Tried to look up a type by name, but there is no type with that name.
    struct TypeNotFound : TypeError {
        String name;
        TypeNotFound (String&& n) : name(n) { }
    };
     // TODO: This is unused, get rid of it
    struct WrongType : TypeError {
        Type expected;
        Type got;
        WrongType (Type e, Type g) : expected(e), got(g) { }
    };
     // Tried to default construct a type that has no default constructor.
    struct CannotDefaultConstruct : TypeError {
        Type type;
        CannotDefaultConstruct (Type t) : type(t) { }
    };
     // Tried to construct or destroy a type that has no destructor.
    struct CannotDestruct : TypeError {
        Type type;
        CannotDestruct (Type t) : type(t) { }
    };
     // Tried to coerce between types that can't be coerced.
    struct CannotCoerce : TypeError {
        Type from;
        Type to;
        CannotCoerce (Type f, Type t) : from(f), to(t) { }
    };
}

} // namespace ayu

 // Allow hashing Type for std::unordered_map
namespace std {
    template <>
    struct hash<ayu::Type> {
        size_t operator () (ayu::Type t) const {
            return hash<void*>()((void*)t.desc);
        }
    };
}

