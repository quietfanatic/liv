#include "tree-private.h"

#include "../compat.h"
#include "../print.h"
#include "../describe.h"

namespace ayu {
using namespace in;

Str form_name (TreeForm f) {
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

Tree::Tree (const Tree& o) :
    form(o.form), rep(o.rep), flags(o.flags), data(o.data)
{
    switch (rep) {
        case REP_STRING:
        case REP_ARRAY:
        case REP_OBJECT:
        case REP_ERROR:
            data.as_ptr->ref_count++;
            break;
    }
}

Tree::~Tree () {
    switch (rep) {
        case REP_STRING: {
            if (!--data.as_ptr->ref_count) {
                delete static_cast<const TreeData<String>*>(data.as_ptr);
            }
            break;
        }
        case REP_ARRAY: {
            if (!--data.as_ptr->ref_count) {
                delete static_cast<const TreeData<Array>*>(data.as_ptr);
            }
            break;
        }
        case REP_OBJECT: {
            if (!--data.as_ptr->ref_count) {
                delete static_cast<const TreeData<Object>*>(data.as_ptr);
            }
            break;
        }
        case REP_ERROR: {
            if (!--data.as_ptr->ref_count) {
                delete static_cast<const TreeData<std::exception_ptr>*>(data.as_ptr);
            }
            break;
        }
    }
}

Tree::Tree (Null) :
    form(NULLFORM), rep(REP_NULL), data{}
{ }
Tree::Tree (ExplicitBool v) :
    form(BOOL), rep(REP_BOOL), data{.as_usize = v.v}
{ }
Tree::Tree (int64 v) :
    form(NUMBER), rep(REP_INT64), data{.as_int64 = v}
{ }
Tree::Tree (double v) :
    form(NUMBER), rep(REP_DOUBLE), data{.as_double = v}
{ }
Tree::Tree (Str v) :
    form(STRING), rep(), data{}
{
    if (v.size() <= 8) {
        const_cast<uint8&>(rep) = REP_0CHARS + v.size();
        for (usize i = 0; i < v.size(); i++) {
            const_cast<char&>(data.as_chars[i]) = v[i];
        }
    }
    else {
        const_cast<uint8&>(rep) = REP_STRING;
        const_cast<RefCounted*&>(data.as_ptr) =
            new TreeData<String>({1}, String(v));
    }
}
Tree::Tree (String&& v) :
    form(STRING), rep(), data{}
{
    if (v.size() <= 8) {
        const_cast<uint8&>(rep) = REP_0CHARS + v.size();
        for (usize i = 0; i < v.size(); i++) {
            const_cast<char&>(data.as_chars[i]) = v[i];
        }
    }
    else {
        const_cast<uint8&>(rep) = REP_STRING;
        const_cast<RefCounted*&>(data.as_ptr) =
            new TreeData<String>({1}, std::move(v));
    }
}
Tree::Tree (String16&& v) : Tree(from_utf16(v)) { }
Tree::Tree (Array v) :
    form(ARRAY), rep(REP_ARRAY), data{
        .as_ptr = new TreeData<Array>({1}, std::move(v))
    }
{ }
Tree::Tree (Object v) :
    form(OBJECT), rep(REP_OBJECT), data{
        .as_ptr = new TreeData<Object>({1}, std::move(v))
    }
{ }
Tree::Tree (std::exception_ptr v) :
    form(ERROR), rep(REP_ERROR), data{
        .as_ptr = new TreeData<std::exception_ptr>({1}, std::move(v))
    }
{ }

static void require_form (const Tree& t, TreeForm form) {
    if (t.rep == REP_ERROR) std::rethrow_exception(tree_Error(t));
    else if (t.form != form) throw X<WrongForm>(form, t);
}

Tree::operator Null () const {
    require_form(*this, NULLFORM);
    return null;
}
Tree::operator bool () const {
    require_form(*this, BOOL);
    return tree_bool(*this);
}
Tree::operator char () const {
    if (rep == REP_1CHARS) {
        return data.as_chars[0];
    }
    else {
        [[unlikely]]
        switch (rep) {
            case REP_ERROR: std::rethrow_exception(tree_Error(*this));
            case REP_0CHARS: case REP_1CHARS: case REP_2CHARS: case REP_3CHARS:
            case REP_4CHARS: case REP_5CHARS: case REP_6CHARS: case REP_7CHARS:
            case REP_8CHARS: case REP_STRING: {
                 // We guarantee in construction that trees with REP_STRING will
                 // never have 8 or less characters.
                throw X<CantRepresent>("char", *this);
            }
            default: throw X<WrongForm>(form, *this);
        }
    }
}
#define INTEGRAL_CONVERSION(T) \
Tree::operator T () const { \
    switch (rep) { \
        case REP_INT64: { \
            int64 v = tree_int64(*this); \
            if (int64(T(v)) == v) return v; \
            else throw X<CantRepresent>(#T, *this); \
        } \
        case REP_DOUBLE: { \
            double v = tree_double(*this); \
            if (double(T(v)) == v) return v; \
            else throw X<CantRepresent>(#T, *this); \
        } \
        case REP_ERROR: { \
            std::rethrow_exception(tree_Error(*this)); \
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
#undef INTEGRAL_CONVERSION
Tree::operator double () const {
    switch (rep) {
         // Special case: allow null to represent +nan for JSON compatibility
        case REP_NULL: return +nan;
        case REP_INT64: return tree_int64(*this);
        case REP_DOUBLE: return tree_double(*this);
        case REP_ERROR: std::rethrow_exception(tree_Error(*this));
        default: throw X<WrongForm>(NUMBER, *this);
    }
}
Tree::operator Str () const {
    switch (rep) {
        case REP_ERROR: std::rethrow_exception(tree_Error(*this));
        case REP_0CHARS: case REP_1CHARS: case REP_2CHARS: case REP_3CHARS:
        case REP_4CHARS: case REP_5CHARS: case REP_6CHARS: case REP_7CHARS:
        case REP_8CHARS: {
            return tree_chars(*this);
        }
        case REP_STRING: {
            return tree_String(*this);
        }
        default: throw X<WrongForm>(STRING, *this);
    }
}
Tree::operator String () const {
    return String(Str(*this));
}
Tree::operator String16 () const {
    return to_utf16(Str(*this));
}
Tree::operator const Array& () const {
    require_form(*this, ARRAY);
    return tree_Array(*this);
}
Tree::operator const Object& () const {
    require_form(*this, OBJECT);
    return tree_Object(*this);
}

const Tree* Tree::attr (Str key) const {
    require_form(*this, OBJECT);
    for (auto& p : tree_Object(*this)) {
        if (p.first == key) return &p.second;
    }
    return null;
}
const Tree* Tree::elem (usize index) const {
    require_form(*this, ARRAY);
    if (index >= tree_Array(*this).size()) return null;
    return &tree_Array(*this)[index];
}
const Tree& Tree::operator[] (Str key) const {
    if (const Tree* r = attr(key)) return *r;
    else throw X<GenericError>(cat(
        "This tree has no attr with key \""sv, key, '"'
    ));
}
const Tree& Tree::operator[] (usize index) const {
    if (const Tree* r = elem(index)) return *r;
    else throw X<GenericError>(cat(
        "This tree has no elem with index \""sv, index, '"'
    ));
}

bool operator == (const Tree& a, const Tree& b) {
     // Shortcut if same address
    if (&a == &b) return true;
     // Special case int/float comparisons
    else if (a.rep == REP_INT64 && b.rep == REP_DOUBLE) {
        return tree_int64(a) == tree_double(b);
    }
    else if (a.rep == REP_DOUBLE && b.rep == REP_INT64) {
        return tree_double(a) == tree_int64(b);
    }
     // Otherwise different reps = different values.  We don't need to compare
     // REP_*CHARS to REP_STRING because we guarantee that REP_STRING will never
     // have 8 or less chars.
    else if (a.rep != b.rep) return false;
    else switch (a.rep) {
        case REP_NULL: return true;
        case REP_BOOL: return tree_bool(a) == tree_bool(b);
        case REP_INT64: return tree_int64(a) == tree_int64(b);
        case REP_DOUBLE: {
            double av = tree_double(a);
            double bv = tree_double(b);
             // Check for nans
            if (av != av && bv != bv) return true;
            else return av == bv;
        }
        case REP_STRING: {
            if (a.data.as_ptr == b.data.as_ptr) return true;
            return tree_String(a) == tree_String(b);
        }
        case REP_ARRAY: {
            if (a.data.as_ptr == b.data.as_ptr) return true;
            return tree_Array(a) == tree_Array(b);
        }
        case REP_OBJECT: {
            if (a.data.as_ptr == b.data.as_ptr) return true;
            const Object& ao = tree_Object(a);
            const Object& bo = tree_Object(b);
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
        case REP_ERROR: return false;
        case REP_0CHARS: case REP_1CHARS: case REP_2CHARS: case REP_3CHARS:
        case REP_4CHARS: case REP_5CHARS: case REP_6CHARS: case REP_7CHARS:
        case REP_8CHARS: {
            return tree_chars(a) == tree_chars(b);
        }
        default: AYU_INTERNAL_UGUU();
    }
}

} using namespace ayu;

AYU_DESCRIBE(ayu::TreeForm,
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
    to_tree([](const Tree& v){ return v; }),
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
    done_testing();
});
#endif
