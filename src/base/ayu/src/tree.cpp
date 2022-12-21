#include "tree-private.h"

#include "../compat.h"
#include "../print.h"
#include "../describe.h"

namespace ayu {
using namespace in;

Tree::Tree (const Tree& o) :
    form(o.form), rep(o.rep), flags(o.flags), data(o.data)
{
    if (rep < 0) {
        data.as_ptr->ref_count++;
    }
}

[[gnu::noinline]]
void delete_data (const Tree& t) {
    switch (t.rep) {
        case REP_VARCHAR: {
            free((void*)t.data.as_ptr);
            break;
        }
        case REP_ARRAY: {
            delete static_cast<const TreeData<Array>*>(t.data.as_ptr);
            break;
        }
        case REP_OBJECT: {
            delete static_cast<const TreeData<Object>*>(t.data.as_ptr);
            break;
        }
        case REP_ERROR: {
            delete static_cast<const TreeData<std::exception_ptr>*>(t.data.as_ptr);
            break;
        }
        default: AYU_INTERNAL_UGUU();
    }
}

Tree::~Tree () {
    if (rep < 0) {
        if (!--data.as_ptr->ref_count) {
            delete_data(*this);
        }
    }
}

Tree::Tree (Null) :
    form(NULLFORM), rep(REP_NULL), data{}
{ }
Tree::Tree (ExplicitBool v) :
    form(BOOL), rep(REP_BOOL), data{.as_bool = v.v}
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
         // zero unused char slots
        const_cast<int64&>(data.as_int64) = 0;
        const_cast<int8&>(rep) = REP_0CHARS + v.size();
        for (usize i = 0; i < v.size(); i++) {
            const_cast<char&>(data.as_chars[i]) = v[i];
        }
    }
    else {
        const_cast<int8&>(rep) = REP_VARCHAR;
        auto vc = (TreeData<VarChar>*)malloc(sizeof(TreeData<VarChar>) + v.size());
        vc->ref_count = 1;
        vc->value.size = v.size();
        for (usize i = 0; i < v.size(); i++) {
            vc->value.data[i] = v[i];
        }
        const_cast<RefCounted*&>(data.as_ptr) = vc;
    }
}
Tree::Tree (String&& v) :
    form(STRING), rep(), data{.as_int64 = 0}
{
    if (v.size() <= 8) {
        const_cast<int8&>(rep) = REP_0CHARS + v.size();
        for (usize i = 0; i < v.size(); i++) {
            const_cast<char&>(data.as_chars[i]) = v[i];
        }
    }
    else {
        const_cast<int8&>(rep) = REP_VARCHAR;
        auto vc = (TreeData<VarChar>*)malloc(sizeof(TreeData<VarChar>) + v.size());
        vc->ref_count = 1;
        vc->value.size = v.size();
        for (usize i = 0; i < v.size(); i++) {
            vc->value.data[i] = v[i];
        }
        const_cast<RefCounted*&>(data.as_ptr) = vc;
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

[[noreturn]]
static void bad_form (const Tree& t, TreeForm form) {
    if (t.rep == REP_ERROR) std::rethrow_exception(tree_Error(t));
    else if (t.form == form) {
        AYU_INTERNAL_UGUU();
    }
    else throw X<WrongForm>(form, t);
}

Tree::operator Null () const {
    if (form != NULLFORM) bad_form(*this, NULLFORM);
    return null;
}
Tree::operator bool () const {
    if (form != BOOL) bad_form(*this, BOOL);
    return data.as_bool;
}
Tree::operator char () const {
    if (rep == REP_1CHARS) {
        return data.as_chars[0];
    }
    else {
        [[unlikely]]
        switch (rep) {
            case REP_0CHARS: case REP_1CHARS: case REP_2CHARS: case REP_3CHARS:
            case REP_4CHARS: case REP_5CHARS: case REP_6CHARS: case REP_7CHARS:
            case REP_8CHARS: case REP_VARCHAR: {
                 // We guarantee in construction that trees with REP_VARCHAR will
                 // never have 8 or less characters.
                throw X<CantRepresent>("char", *this);
            }
            default: bad_form(*this, STRING);
        }
    }
}
#define INTEGRAL_CONVERSION(T) \
Tree::operator T () const { \
    switch (rep) { \
        case REP_INT64: { \
            int64 v = data.as_int64; \
            if (int64(T(v)) == v) return v; \
            else throw X<CantRepresent>(#T, *this); \
        } \
        case REP_DOUBLE: { \
            double v = data.as_double; \
            if (double(T(v)) == v) return v; \
            else throw X<CantRepresent>(#T, *this); \
        } \
        default: bad_form(*this, NUMBER); \
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
        case REP_INT64: return data.as_int64;
        case REP_DOUBLE: return data.as_double;
        default: bad_form(*this, NUMBER);
    }
}
Tree::operator Str () const {
    switch (rep) {
        case REP_0CHARS: case REP_1CHARS: case REP_2CHARS: case REP_3CHARS:
        case REP_4CHARS: case REP_5CHARS: case REP_6CHARS: case REP_7CHARS:
        case REP_8CHARS: {
            return tree_shortStr(*this);
        }
        case REP_VARCHAR: {
            return tree_longStr(*this);
        }
        default: bad_form(*this, STRING);
    }
}
Tree::operator String () const {
    return String(Str(*this));
}
Tree::operator String16 () const {
    return to_utf16(Str(*this));
}
Tree::operator const Array& () const {
    if (rep != REP_ARRAY) bad_form(*this, ARRAY);
    return tree_Array(*this);
}
Tree::operator const Object& () const {
    if (rep != REP_OBJECT) bad_form(*this, OBJECT);
    return tree_Object(*this);
}

const Tree* Tree::attr (Str key) const {
    if (rep != REP_OBJECT) bad_form(*this, OBJECT);
    for (auto& p : tree_Object(*this)) {
        if (p.first == key) return &p.second;
    }
    return null;
}
const Tree* Tree::elem (usize index) const {
    if (rep != REP_ARRAY) bad_form(*this, ARRAY);
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
    else if (a.rep != b.rep) {
         // Special case int/float comparisons
        if (a.rep == REP_INT64 && b.rep == REP_DOUBLE) {
            return a.data.as_int64 == b.data.as_double;
        }
        else if (a.rep == REP_DOUBLE && b.rep == REP_INT64) {
            return a.data.as_double == b.data.as_int64;
        }
         // Otherwise different reps = different values.  We don't need to compare
         // REP_*CHARS to REP_STRING because we guarantee that REP_STRING will never
         // have 8 or less chars.
        return false;
    }
    else switch (a.rep) {
        case REP_NULL: return true;
        case REP_BOOL: return a.data.as_bool == b.data.as_bool;
        case REP_INT64: return a.data.as_int64 == b.data.as_int64;
        case REP_DOUBLE: {
            double av = a.data.as_double;
            double bv = b.data.as_double;
            return av == bv || (av != av && bv != bv);
        }
        case REP_VARCHAR: {
            if (a.data.as_ptr == b.data.as_ptr) return true;
            return tree_longStr(a) == tree_longStr(b);
        }
        case REP_ARRAY: {
             // From my investigations, the STL does NOT, in general,
             // short-circuit container comparisons where the containers have
             // the same address.
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
             // Unused char slots are zeroed out so we can compare them all at
             // once.
            return a.data.as_int64 == b.data.as_int64;
        }
        default: AYU_INTERNAL_UGUU();
    }
}

} using namespace ayu;

AYU_DESCRIBE(ayu::TreeForm,
    values(
        value("undefined", UNDEFINED),
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
