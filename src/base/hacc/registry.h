#pragma once

 // These things are kind of a nexus of dependency, so let's quarantine
 //  them into their own file.

#include <typeinfo>

#include "common.h"

namespace haccable {
    template <class T>
    struct Haccability {
        static constexpr bool defined = false;
    };
}

namespace hacc::X {
     // TODO: serializing this doesn't work?
    struct Unhaccable : LogicError {
        const std::type_info& cpp_type;
        Unhaccable (const std::type_info& t) : cpp_type(t) { }
    };
    struct TypeNotFound : LogicError {
        String name;
        TypeNotFound (String&& n) : name(n) { }
    };
}

namespace hacc::in {
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
        if constexpr (haccable::Haccability<T>::defined) {
            return haccable::Haccability<T>::description;
        }
        else {
            return get_description_by_type_info(typeid(T));
        }
    }
    template <class T>
    const Description* need_description_for_cpp_type () {
        if constexpr (haccable::Haccability<T>::defined) {
            return haccable::Haccability<T>::description;
        }
        else {
            return need_description_for_type_info(typeid(T));
        }
    }
    std::string get_demangled_name (const std::type_info& t);
}

