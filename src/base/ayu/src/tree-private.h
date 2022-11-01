#pragma once

#include "../tree.h"

#include "../../tap/tap.h"
#include "../print.h"

namespace ayu::in {

enum class Rep : uint8 {
    UNDEFINED,
    NULLREP,
    BOOL,
    INT64,
    DOUBLE,
    STRING,
    ARRAY,
    OBJECT
};

constexpr Form form_of_rep (Rep rep) {
    switch (rep) {
        case Rep::NULLREP: return NULLFORM;
        case Rep::BOOL: return BOOL;
        case Rep::INT64: return NUMBER;
        case Rep::DOUBLE: return NUMBER;
        case Rep::STRING: return STRING;
        case Rep::ARRAY: return ARRAY;
        case Rep::OBJECT: return OBJECT;
        default: AYU_INTERNAL_ERROR();
    }
}

template <class T> constexpr Rep rep_of_type;
template <> constexpr Rep rep_of_type<Null> = Rep::NULLREP;
template <> constexpr Rep rep_of_type<bool> = Rep::BOOL;
template <> constexpr Rep rep_of_type<int64> = Rep::INT64;
template <> constexpr Rep rep_of_type<double> = Rep::DOUBLE;
template <> constexpr Rep rep_of_type<String> = Rep::STRING;
template <> constexpr Rep rep_of_type<Array> = Rep::ARRAY;
template <> constexpr Rep rep_of_type<Object> = Rep::OBJECT;

template <class T> constexpr Form form_of_type = form_of_rep(rep_of_type<T>);

template <class T> struct TreeDataT;

struct TreeData : RefCounted {
    Rep rep;

    TreeData (Rep r, uint32 rc = 0) :
        RefCounted{rc}, rep(r)
    { }

    template <class T>
    T& as_known () { return static_cast<TreeDataT<T>*>(this)->value; }
    template <class T>
    T& as () {
        if (rep_of_type<T> == rep) return as_known<T>();
        else throw X::WrongForm(form_of_type<T>, Tree(this));
    }
};

template <class T>
struct TreeDataT : TreeData {
    T value;
    TreeDataT (const T& v, uint32 rc = 0) :
        TreeData(rep_of_type<T>, rc), value(v)
    { }
    TreeDataT (T&& v, uint32 rc = 0) :
        TreeData(rep_of_type<T>, rc), value(std::move(v))
    { }
};

} // namespace ayu::in

namespace tap {
    template <>
    struct Show<ayu::Tree> {
        std::string show (const ayu::Tree& t) {
            return tree_to_string(t, ayu::COMPACT);
        }
    };
}
