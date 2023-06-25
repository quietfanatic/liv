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
        static constexpr bool _ayu_is_local = false;
         // Declare this but don't define it.  It will be defined in a
         // specialization of this template, which may be in a different
         // translation unit.  Apparently nobody knows whether that's legal or
         // not, but it works as long as the compiler uses the same mangled
         // names for the specialization as the prototype.
        static const ayu::in::Description* const _ayu_description;
    };
}

namespace ayu::in {
    const Description* register_description (const Description*);
    const Description* get_description_for_type_info (const std::type_info&);
    const Description* need_description_for_type_info (const std::type_info&);
    const Description* get_description_for_name (Str);
    const Description* need_description_for_name (Str);
    [[noreturn]] void throw_UnknownType (const std::type_info&);

    Str get_description_name (const Description*);
     // If this returns false, the type is probably a corrupted pointer and
     // shouldn't be dereferenced.
    bool is_valid_type (const Description*);

    template <class T> requires (!std::is_reference_v<T>)
    constexpr const Description* const* get_indirect_description () {
        return &ayu_desc::_AYU_Describe<std::remove_cv_t<T>>::_ayu_description;
    }

    template <class T> requires (!std::is_reference_v<T>)
    const Description* get_description_for_cpp_type () {
        return ayu_desc::_AYU_Describe<std::remove_cv_t<T>>::_ayu_description;
    }
}

