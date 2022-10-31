// This module implements the central registry of all described types in the
// program.  This is kind of a nexus of dependency, so we're keeping it in its
// own module.
// TODO: move to internal folder

#pragma once

#include <typeinfo>

#include "common.h"

 // I was going to use ayu::desc here but using a nested namespace seems to
 // cause weird errors in some situations.
namespace ayu_desc {
    template <class T>
    struct Describe {
        static constexpr bool defined = false;
    };
}

 // TODO: Move these to type.h
namespace ayu::X {
     // Tried to map a C++ type to an AYU type, but AYU doesn't know about this
     // type (it has no AYU_DESCRIBE description).
     // TODO: serializing this doesn't work?
    struct UnknownType : LogicError {
        const std::type_info& cpp_type;
        UnknownType (const std::type_info& t) : cpp_type(t) { }
    };
     // Tried to look up a type by name, but there is no type with that name.
    struct TypeNotFound : LogicError {
        String name;
        TypeNotFound (String&& n) : name(n) { }
    };
}

namespace ayu::in {
    const Description* register_description (const Description*);
    const Description* get_description_by_type_info (const std::type_info&);
    const Description* need_description_for_type_info (const std::type_info&);
    const Description* get_description_by_name (Str);
    const Description* need_description_for_name (Str);
    void dump_descriptions ();

    Str get_description_name (const Description*);
     // If this returns false, the type is probably a corrupted pointer and
     //  shouldn't be dereferenced.
    bool is_valid_type (const Description*);

    template <class T>
    const Description* get_description_by_cpp_type () {
        if constexpr (ayu_desc::Describe<T>::defined) {
            return ayu_desc::Describe<T>::description;
        }
        else {
            return get_description_by_type_info(typeid(T));
        }
    }
    template <class T>
    const Description* need_description_for_cpp_type () {
        if constexpr (ayu_desc::Describe<T>::defined) {
            return ayu_desc::Describe<T>::description;
        }
        else {
            return need_description_for_type_info(typeid(T));
        }
    }
    std::string get_demangled_name (const std::type_info& t);
}

