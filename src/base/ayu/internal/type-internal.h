// This module implements the central registry of all described types in the
// program.  This is kind of a nexus of dependency, so we're keeping it in its
// own module.

#pragma once

#include <typeinfo>

#include "../common.h"

 // I was going to use ayu::desc here but using a nested namespace seems to
 // cause weird errors in some situations.  Besides, having the namespace nested
 // in ayu:: automatically makes names in ayu:: visible, which may not be
 // desired.
namespace ayu_desc {
    template <class T>
    struct _AYU_Describe {
        static constexpr bool _ayu_defined = false;
    };
}

namespace ayu::in {
    const Description* register_description (const Description*);
    const Description* get_description_by_type_info (const std::type_info&);
    const Description* need_description_for_type_info (const std::type_info&);
    const Description* get_description_by_name (Str);
    const Description* need_description_for_name (Str);
    [[noreturn]] void throw_UnknownType (const std::type_info&);

    Str get_description_name (const Description*);
     // If this returns false, the type is probably a corrupted pointer and
     // shouldn't be dereferenced.
    bool is_valid_type (const Description*);

    template <class T> requires (
        !std::is_reference_v<T> && !std::is_const_v<T> && !std::is_volatile_v<T>
    )
    const Description* get_description_by_cpp_type () {
        if constexpr (ayu_desc::_AYU_Describe<T>::_ayu_defined) {
            return ayu_desc::_AYU_Describe<T>::_ayu_description;
        }
        else {
            static auto r = get_description_by_type_info(typeid(T));
            return r;
        }
    }
    template <class T> requires (
        !std::is_reference_v<T> && !std::is_const_v<T> && !std::is_volatile_v<T>
    )
    const Description* need_description_for_cpp_type () {
        if constexpr (ayu_desc::_AYU_Describe<T>::_ayu_defined) {
            return ayu_desc::_AYU_Describe<T>::_ayu_description;
        }
        else {
            if (auto r = get_description_by_cpp_type<T>()) {
                return r;
            }
            else throw_UnknownType(typeid(T));
        }
    }
    std::string get_demangled_name (const std::type_info& t);
}

