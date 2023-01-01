#include "tree-private.h"

#include <cstring>
#include "../../uni/utf.h"
#include "../print.h"
#include "../describe.h"

namespace ayu {
using namespace in;

Tree::Tree (const Tree& o) :
    form(o.form), rep(o.rep), flags(o.flags), length(o.length), data(o.data)
{
    if (rep < 0 && data.as_char_ptr) {
        ++ArrayOwnedHeader::get(data.as_char_ptr)->ref_count;
    }
}

[[gnu::noinline]]
void delete_data (TreeRef t) {
     // Delete by manifesting an array and letting its destructor run.
    switch (t->rep) {
        case REP_LONGSTRING: {
            UniqueArray<char>::Materialize(
                (char*)t->data.as_char_ptr, t->length
            );
            break;
        }
        case REP_ARRAY: {
            UniqueArray<Tree>::Materialize(
                (Tree*)t->data.as_array_ptr, t->length
            );
            break;
        }
        case REP_OBJECT: {
            UniqueArray<TreePair>::Materialize(
                (TreePair*)t->data.as_object_ptr, t->length
            );
            break;
        }
        case REP_ERROR: {
            UniqueArray<std::exception_ptr>::Materialize(
                (std::exception_ptr*)t->data.as_error_ptr, t->length
            );
            break;
        }
        default: never();
    }
}

Tree::~Tree () {
    if (rep < 0 && data.as_char_ptr) {
        auto header = ArrayOwnedHeader::get(data.as_char_ptr);
        if (header->ref_count) {
            --header->ref_count;
        }
        else {
            delete_data(*this);
        }
    }
}

Tree::Tree (Null) :
    form(NULLFORM), rep(REP_NULL), flags(0), length(0), data{.as_int64 = 0}
{ }
 // use .as_int64 to write all of data
Tree::Tree (ExplicitBool v) :
    form(BOOL), rep(REP_BOOL), flags(0), length(0), data{.as_int64 = v.v}
{ }
Tree::Tree (int64 v) :
    form(NUMBER), rep(REP_INT64), flags(0), length(0), data{.as_int64 = v}
{ }
Tree::Tree (double v) :
    form(NUMBER), rep(REP_DOUBLE), flags(0), length(0), data{.as_double = v}
{ }
Tree::Tree (GenericStr<char> v) :
    form(STRING), rep(v.size() <= 8 ? REP_SHORTSTRING : REP_LONGSTRING),
    flags(0), length(v.size()), data{}
{
    require(v.size() <= uint32(-1));
    if (v.size() <= 8) {
         // zero unused char slots
        const_cast<int64&>(data.as_int64) = 0;
        for (usize i = 0; i < v.size(); i++) {
            const_cast<char&>(data.as_chars[i]) = v[i];
        }
    }
    else {
        auto s = UniqueString::Copy(v);
        const_cast<char*&>(data.as_char_ptr) = s.data();
        s.dematerialize();
    }
}
Tree::Tree (Str16 v) : Tree(from_utf16(v)) { }
Tree::Tree (SharedArray<char> v) :
    form(STRING), rep(v.size() <= 8 ? REP_SHORTSTRING : REP_LONGSTRING),
    flags(0), length(v.size()), data{}
{
    require(v.size() <= uint32(-1));
    if (v.size() <= 8) {
        const_cast<int64&>(data.as_int64) = 0;
        for (usize i = 0; i < v.size(); i++) {
            const_cast<char&>(data.as_chars[i]) = v[i];
        }
    }
    else {
        const_cast<const char*&>(data.as_char_ptr) = v.data();
        v.dematerialize();
    }
}
Tree::Tree (TreeArray v) :
    form(ARRAY), rep(REP_ARRAY), flags(0),
    length(v.size()), data{.as_array_ptr = v.data()}
{
    require(v.size() <= uint32(-1));
    v.dematerialize();
}
Tree::Tree (TreeObject v) :
    form(OBJECT), rep(REP_OBJECT), flags(0),
    length(v.size()), data{.as_object_ptr = v.data()}
{
    require(v.size() <= uint32(-1));
    v.dematerialize();
}
Tree::Tree (std::exception_ptr v) :
    form(ERROR), rep(REP_ERROR), flags(0), length(1), data{}
{
    auto e = UniqueArray<std::exception_ptr>(1, v);
    const_cast<const std::exception_ptr*&>(data.as_error_ptr) = e.data();
    e.dematerialize();
}

[[noreturn]]
static void bad_form (TreeRef t, TreeForm form) {
    if (t->rep == REP_ERROR) std::rethrow_exception(tree_Error(t));
    else if (t->form == form) never();
    else throw X<WrongForm>(form, t);
}

Tree::operator Null () const {
    if (rep != REP_NULL) bad_form(*this, NULLFORM);
    return null;
}
Tree::operator bool () const {
    if (rep != REP_BOOL) bad_form(*this, BOOL);
    return data.as_bool;
}
Tree::operator char () const {
    if (rep == REP_SHORTSTRING) {
        return data.as_chars[0];
    }
    else [[unlikely]] {
        if (rep == REP_LONGSTRING) throw X<CantRepresent>("char", *this);
        else bad_form(*this, STRING);
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
        case REP_SHORTSTRING: return tree_shortStr(*this);
        case REP_LONGSTRING: return tree_longStr(*this);
        default: bad_form(*this, STRING);
    }
}
Tree::operator std::string () const {
    return std::string(Str(*this));
}
Tree::operator std::u16string () const {
    return to_utf16(Str(*this));
}
Tree::operator TreeArraySlice () const {
    if (rep != REP_ARRAY) bad_form(*this, ARRAY);
    return tree_Array(*this);
}
Tree::operator TreeArray () const {
    if (rep != REP_ARRAY) bad_form(*this, ARRAY);
    return TreeArray(tree_Array(*this));
}
Tree::operator TreeObjectSlice () const {
    if (rep != REP_OBJECT) bad_form(*this, OBJECT);
    return tree_Object(*this);
}
Tree::operator TreeObject () const {
    if (rep != REP_OBJECT) bad_form(*this, OBJECT);
    return TreeObject(tree_Object(*this));
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
    else throw X<GenericError>(old_cat(
        "This tree has no attr with key \""sv, key, '"'
    ));
}
const Tree& Tree::operator[] (usize index) const {
    if (const Tree* r = elem(index)) return *r;
    else throw X<GenericError>(old_cat(
        "This tree has no elem with index \""sv, index, '"'
    ));
}

bool operator == (TreeRef a, TreeRef b) {
    if (a->rep != b->rep) {
         // Special case int/float comparisons
        if (a->rep == REP_INT64 && b->rep == REP_DOUBLE) {
            return a->data.as_int64 == b->data.as_double;
        }
        else if (a->rep == REP_DOUBLE && b->rep == REP_INT64) {
            return a->data.as_double == b->data.as_int64;
        }
         // Otherwise different reps = different values.  We don't need to
         // compare REP_SHORTSTRING to REP_LONGSTRING because we guarantee that
         // REP_LONGSTRING has at least 9 characters.
        return false;
    }
    else switch (a->rep) {
        case REP_NULL: return true;
        case REP_BOOL: return a->data.as_bool == b->data.as_bool;
        case REP_INT64: return a->data.as_int64 == b->data.as_int64;
        case REP_DOUBLE: {
            double av = a->data.as_double;
            double bv = b->data.as_double;
            return av == bv || (av != av && bv != bv);
        }
        case REP_SHORTSTRING: {
            if (a->length != b->length) return false;
             // Unused char slots are zeroed out so we can compare them all at
             // once.
            return a->data.as_int64 == b->data.as_int64;
        }
        case REP_LONGSTRING: {
            return tree_longStr(a) == tree_longStr(b);
        }
        case REP_ARRAY: {
            return tree_Array(a) == tree_Array(b);
        }
        case REP_OBJECT: {
            TreeObjectSlice ao = tree_Object(a);
            TreeObjectSlice bo = tree_Object(b);
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
        default: never();
    }
}

bool operator == (TreeRef a, Str b) {
    if (a->length != b.size()) return false;
    if (b.size() <= 8) {
        if (a->rep != REP_SHORTSTRING) return false;
        if (a->data.as_chars == b.data()) return true;
        for (uint32 i = 0; i < b.size(); i++) {
            if (a->data.as_chars[i] != b[i]) return false;
        }
        return true;
    }
    else {
        if (a->rep != REP_LONGSTRING) return false;
        return Slice<char>(tree_longStr(a)) == Slice<char>(b);
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
    is(Str(Tree("asdfg"sv)), "asdfg"sv, "Round-trip strings");
    is(Str(Tree("qwertyuiop"sv)), "qwertyuiop"sv, "Round-trip long strings");
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
    is(Tree(TreeArray{Tree(3), Tree(4)}), Tree(TreeArray{Tree(3), Tree(4)}), "Compare arrays.");
    isnt(Tree(TreeArray{Tree(3), Tree(4)}), Tree(TreeArray{Tree(4), Tree(3)}), "Compare unequal arrays.");
    is(
        Tree(TreeObject{TreePair{"a", Tree(0)}, TreePair{"b", Tree(1)}}),
        Tree(TreeObject{TreePair{"b", Tree(1)}, TreePair{"a", Tree(0)}}),
        "TreeObject with same attributes in different order are equal"
    );
    isnt(
        Tree(TreeObject{TreePair{"a", Tree(0)}, TreePair{"b", Tree(1)}}),
        Tree(TreeObject{TreePair{"b", Tree(1)}, TreePair{"a", Tree(0)}, TreePair{"c", Tree(3)}}),
        "Extra attribute in second object makes it unequal"
    );
    done_testing();
});
#endif
