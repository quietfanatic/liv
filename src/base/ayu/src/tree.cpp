#include "tree-private.h"

#include "../compat.h"
#include "../print.h"
#include "../describe.h"

namespace ayu {
using namespace in;

Str form_name (Form f) {
    switch (f) {
        case NULLFORM: return "null"sv;
        case BOOL: return "bool"sv;
        case NUMBER: return "number"sv;
        case STRING: return "string"sv;
        case ARRAY: return "array"sv;
        case OBJECT: return "object"sv;
        case ERROR: return "error"sv;
        default: return "(invalid form ID)"sv;
    }
}

namespace in {
    void delete_TreeData (TreeData* data) {
        switch (data->rep) {
             // Only bothering with cases where the data needs destructing.
            case Rep::STRING: delete static_cast<TreeDataT<String>*>(data); break;
            case Rep::ARRAY: delete static_cast<TreeDataT<Array>*>(data); break;
            case Rep::OBJECT: delete static_cast<TreeDataT<Object>*>(data); break;
            case Rep::ERROR: delete static_cast<TreeDataT<std::exception_ptr>*>(data); break;
            default: delete data; break;
        }
    }
}

Form Tree::form () const { return form_of_rep(data->rep); }
TreeFlags Tree::flags () const { return data->flags; }

static TreeDataT<Null> global_null {null, 0, 1};
static TreeDataT<bool> global_false {false, 0, 1};
static TreeDataT<bool> global_true {true, 0, 1};
static TreeDataT<int64> global_ints [16] = {
    {-8, 0, 1},
    {-7, 0, 1},
    {-6, 0, 1},
    {-5, 0, 1},
    {-4, 0, 1},
    {-3, 0, 1},
    {-2, 0, 1},
    {-1, 0, 1},
    {0, 0, 1},
    {1, 0, 1},
    {2, 0, 1},
    {3, 0, 1},
    {4, 0, 1},
    {5, 0, 1},
    {6, 0, 1},
    {7, 0, 1}
};
static TreeDataT<double> global_nan {nan, 0, 1};
static TreeDataT<double> global_plus_inf {inf, 0, 1};
static TreeDataT<double> global_minus_inf {-inf, 0, 1};
static TreeDataT<double> global_minus_zero {-0.0, 0, 1};
static TreeDataT<String> global_empty_string {""s, 0, 1};
static TreeDataT<Array> global_empty_array {Array{}, 0, 1};
static TreeDataT<Object> global_empty_object {Object{}, 0, 1};

Tree::Tree (Null, TreeFlags) : Tree(&global_null) { }
namespace in {
    TreeData* TreeData_bool (bool v, TreeFlags) {
        return v ? &global_true : &global_false;
    }
}
Tree::Tree (int64 v, TreeFlags flags) : Tree(
    flags == 0 && v >= -8 && v < 8
        ? &global_ints[v+8]
        : new TreeDataT<int64>(v, flags)
) { }
Tree::Tree (double v, TreeFlags flags) : Tree(
    flags != 0 ? static_cast<TreeData*>(new TreeDataT<double>(v, flags))
  : v != v ? static_cast<TreeData*>(&global_nan)
  : v == inf ? static_cast<TreeData*>(&global_plus_inf)
  : v == -inf ? static_cast<TreeData*>(&global_minus_inf)
  : v == 0 && (1.0/v == -inf) ? static_cast<TreeData*>(&global_minus_zero)
  : int64(v) == v && int64(v) >= -8 && int64(v) < 8
      ? static_cast<TreeData*>(&global_ints[int64(v)+8])
      : static_cast<TreeData*>(new TreeDataT<double>(v, flags))
) { }
Tree::Tree (String&& v, TreeFlags flags) : Tree(
    v.empty() ? &global_empty_string
              : new TreeDataT<String>(std::move(v), flags)
) { }
Tree::Tree (String16&& v, TreeFlags flags) : Tree(
    v.empty() ? &global_empty_string
              : new TreeDataT<String>(from_utf16(v), flags)
) { }
Tree::Tree (const Array& v, TreeFlags flags) : Tree(
    v.empty() ? &global_empty_array
              : new TreeDataT<Array>(v, flags)
) { }
Tree::Tree (Array&& v, TreeFlags flags) : Tree(
    v.empty() ? &global_empty_array
              : new TreeDataT<Array>(std::move(v), flags)
) { }
Tree::Tree (const Object& v, TreeFlags flags) : Tree(
    v.empty() ? &global_empty_object
              : new TreeDataT<Object>(v, flags)
) { }
Tree::Tree (Object&& v, TreeFlags flags) : Tree(
    v.empty() ? &global_empty_object
              : new TreeDataT<Object>(std::move(v), flags)
) { }

Tree::operator Null () const { return data->as<Null>(); }
Tree::operator bool () const { return data->as<bool>(); }
Tree::operator char () const {
    const String& s = data->as<String>();
    if (s.size() == 1) return s[0];
    else throw X<CantRepresent>("char", *this);
}
#define INTEGRAL_CONVERSION(T) \
Tree::operator T () const { \
    switch (data->rep) { \
        case Rep::INT64: { \
            int64 v = data->as_known<int64>(); \
            if (int64(T(v)) == v) return v; \
            else throw X<CantRepresent>(#T, *this); \
        } \
        case Rep::DOUBLE: { \
            double v = data->as_known<double>(); \
            if (double(T(v)) == v) return v; \
            else throw X<CantRepresent>(#T, *this); \
        } \
        case Rep::ERROR: { \
            std::rethrow_exception(data->as_known<std::exception_ptr>()); \
        } \
        default: throw X<WrongForm>(NUMBER, *this); \
    } \
}
INTEGRAL_CONVERSION(int8)
INTEGRAL_CONVERSION(uint8)
INTEGRAL_CONVERSION(int16)
INTEGRAL_CONVERSION(uint16)
INTEGRAL_CONVERSION(int32)
INTEGRAL_CONVERSION(uint32)
INTEGRAL_CONVERSION(int64)
INTEGRAL_CONVERSION(uint64)
Tree::operator double () const {
    switch (data->rep) {
         // Special case: allow null to represent +nan for JSON compatibility
        case Rep::NULLREP: return nan;
        case Rep::INT64: return data->as_known<int64>();
        case Rep::DOUBLE: return data->as_known<double>();
        case Rep::ERROR: {
            rethrow_exception(data->as_known<std::exception_ptr>());
        }
        default: throw X<WrongForm>(NUMBER, *this);
    }
}
Tree::operator Str () const { return data->as<String>(); }
Tree::operator String () const { return data->as<String>(); }
Tree::operator String16 () const { return to_utf16(data->as<String>()); }
Tree::operator const Array& () const { return data->as<Array>(); }
Tree::operator const Object& () const { return data->as<Object>(); }

Tree* Tree::attr (Str key) const {
    for (auto& p : data->as<Object>()) {
        if (p.first == key) return &p.second;
    }
    return null;
}
Tree* Tree::elem (usize index) const {
    if (index >= data->as<Array>().size()) return null;
    return &data->as_known<Array>()[index];
}
Tree Tree::operator[] (Str key) const {
    if (Tree* r = attr(key)) return *r;
    else throw X<GenericError>{cat(
        "This tree has no attr with key \""sv, key, '"'
    )};
}
Tree Tree::operator[] (usize index) const {
    if (Tree* r = elem(index)) return *r;
    else throw X<GenericError>{cat(
        "This tree has no elem with index \""sv, index, '"'
    )};
}

bool operator == (const Tree& a, const Tree& b) {
    auto& ad = *a.data;
    auto& bd = *b.data;
     // Shortcut if same address
    if (&ad == &bd) return true;
     // Special case int/float comparisons
    else if (ad.rep == Rep::INT64 && bd.rep == Rep::DOUBLE) {
        return ad.as_known<int64>() == bd.as_known<double>();
    }
    else if (ad.rep == Rep::DOUBLE && bd.rep == Rep::INT64) {
        return ad.as_known<double>() == bd.as_known<int64>();
    }
     // Otherwise different reps = different values
    else if (ad.rep != bd.rep) return false;
    else switch (ad.rep) {
        case Rep::NULLREP: return true;
        case Rep::BOOL: return ad.as_known<bool>() == bd.as_known<bool>();
        case Rep::INT64: return ad.as_known<int64>() == bd.as_known<int64>();
        case Rep::DOUBLE: {
            double af = ad.as_known<double>();
            double bf = bd.as_known<double>();
             // Check for nans
            if (af != af && bf != bf) return true;
            else return af == bf;
        }
        case Rep::STRING: return ad.as_known<String>() == bd.as_known<String>();
        case Rep::ARRAY: return ad.as_known<Array>() == bd.as_known<Array>();
        case Rep::OBJECT: {
           const Object& ao = ad.as_known<Object>();
           const Object& bo = bd.as_known<Object>();
           if (ao.size() != bo.size()) return false;
           else for (auto& ap : ao) {
               for (auto& bp : bo) {
                   if (ap.first == bp.first) {
                       if (ap.second == bp.second) break;
                       else return false;
                   }
               }
           }
           return true;
        }
        case Rep::ERROR: {
            std::rethrow_exception(ad.as_known<std::exception_ptr>());
        }
        default: AYU_INTERNAL_UGUU();
    }
}

} using namespace ayu;

AYU_DESCRIBE(ayu::Form,
    values(
        value("null", NULLFORM),
        value("bool", BOOL),
        value("number", NUMBER),
        value("string", STRING),
        value("array", ARRAY),
        value("object", OBJECT),
        value("error", ERROR)
    )
)

 // TODO: Add attrs and elems?
AYU_DESCRIBE(ayu::Tree,
    to_tree([](const Tree& v, TreeFlags){ return v; }),
    from_tree([](Tree& v, const Tree& t){ v = t; })
)

AYU_DESCRIBE(ayu::TreeError,
    delegate(base<Error>())
)

AYU_DESCRIBE(ayu::WrongForm,
    elems(
        elem(base<TreeError>(), inherit),
        elem(&WrongForm::form),
        elem(&WrongForm::tree)
    )
)

AYU_DESCRIBE(ayu::CantRepresent,
    elems(
        elem(base<TreeError>(), inherit),
        elem(&CantRepresent::type_name),
        elem(&CantRepresent::tree)
    )
)

#ifndef TAP_DISABLE_TESTS
#include "../../tap/tap.h"

static tap::TestSet tests ("base/ayu/tree", []{
    using namespace tap;
    isnt(Tree(null), Tree(0), "Comparisons fail on different types");
    is(Tree(3), Tree(3.0), "Compare integers with floats");
    isnt(Tree(3), Tree(3.1), "Compare integers with floats (!=)");
    is(Tree(0.0/0.0), Tree(0.0/0.0), "Tree of NAN equals Tree of NAN");
    is(Str(Tree("asdf")), "asdf"sv, "Round-trip strings");
    throws<WrongForm>([]{ int(Tree("0")); }, "Can't convert string to integer");
    try_is<int>([]{ return int(Tree(3.0)); }, 3, "Convert floating to integer");
    try_is<double>([]{ return double(Tree(3)); }, 3.0, "Convert integer to floating");
    throws<CantRepresent>([]{
        int(Tree(3.5));
    }, "Can't convert 3.5 to integer");
    throws<CantRepresent>([]{
        int8(Tree(1000));
    }, "Can't convert 1000 to int8");
    throws<CantRepresent>([]{
        uint8(Tree(-1));
    }, "Can't convert -1 to uint8");
    is(Tree(Array{Tree(3), Tree(4)}), Tree(Array{Tree(3), Tree(4)}), "Compare arrays.");
    isnt(Tree(Array{Tree(3), Tree(4)}), Tree(Array{Tree(4), Tree(3)}), "Compare unequal arrays.");
    is(
        Tree(Object{Pair{"a", Tree(0)}, Pair{"b", Tree(1)}}),
        Tree(Object{Pair{"b", Tree(1)}, Pair{"a", Tree(0)}}),
        "Object with same attributes in different order are equal"
    );
    isnt(
        Tree(Object{Pair{"a", Tree(0)}, Pair{"b", Tree(1)}}),
        Tree(Object{Pair{"b", Tree(1)}, Pair{"a", Tree(0)}, Pair{"c", Tree(3)}}),
        "Extra attribute in second object makes it unequal"
    );
    is(Tree(0xdeadbeef, PREFER_HEX).flags(), PREFER_HEX, "Basic flags support");
    done_testing();
});
#endif
