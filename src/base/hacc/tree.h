 // This defines the main Tree datatype which represents a Hacc structure.
 // Trees are immutable and reference-counted, so copying is cheap, but they
 // can't be accessed on multiple threads at a time.

#pragma once

#include "common.h"

namespace hacc {

 // For unambiguity, types of trees are called forms.
enum Form {
    NULLFORM,
    BOOL,
    NUMBER,
    STRING,
    ARRAY,
    OBJECT,
};
 // Readable name of a form in lowercase.
Str form_name (Form);

struct Tree {
    in::RCP<in::TreeData, in::delete_TreeData> data;
    Tree (in::TreeData* data = null) : data(data) { }
    bool has_value () const { return !!data; }

    explicit Tree (Null v);
     // Disable implicit coercion to bool
    template <class T, std::enable_if_t<std::is_same_v<std::decay_t<T>, bool>, bool> = true>
    explicit Tree (T v);
     // plain (not signed or unsigned) chars are represented as strings
    explicit Tree (char v) : Tree(String(1,v)) { }
    explicit Tree (int8 v) : Tree(int64(v)) { }
    explicit Tree (uint8 v) : Tree(int64(v)) { }
    explicit Tree (int16 v) : Tree(int64(v)) { }
    explicit Tree (uint16 v) : Tree(int64(v)) { }
    explicit Tree (int32 v) : Tree(int64(v)) { }
    explicit Tree (uint32 v) : Tree(int64(v)) { }
    explicit Tree (int64 v);
    explicit Tree (uint64 v) : Tree(int64(v)) { }
    explicit Tree (float v) : Tree(double(v)) { }
    explicit Tree (double v);
    explicit Tree (Str v) : Tree(String(v)) { }
    explicit Tree (String&& v);
    explicit Tree (const char* v) : Tree(String(v)) { }
    explicit Tree (const Array& v);
    explicit Tree (Array&& v);
    explicit Tree (const Object& v);
    explicit Tree (Object&& v);

    Form form () const;

     // These throw if the tree is not the right form or if
     // the requested type cannot store the value.
    explicit operator Null () const;
    explicit operator bool () const;
    explicit operator char () const;
    explicit operator int8 () const;
    explicit operator uint8 () const;
    explicit operator int16 () const;
    explicit operator uint16 () const;
    explicit operator int32 () const;
    explicit operator uint32 () const;
    explicit operator int64 () const;
    explicit operator uint64 () const;
    explicit operator float () const { return double(*this); }
    explicit operator double () const;
    explicit operator Str () const;
    explicit operator String () const;  // Does a copy
    explicit operator const Array& () const;
    explicit operator const Object& () const;

     // Returns null if the invocant is not an OBJECT or does not have an
     //  attribute with the given key.
    Tree* attr (Str key) const;
     // Returns null if the invocant is not an ARRAY or does not have an
     //  element at the given index.
    Tree* elem (usize index) const;

     // Throws if the tree is not an object or doesn't have that attribute.
    Tree operator[] (Str key) const;
     // Throws if the tree is not an array or the index is out of bounds.
    Tree operator[] (usize index) const;
};

 // Test for equality.  Trees of disparate forms are considered unequal.
 //  Objects are equal if all their attributes are the same; the attributes
 //  don't have to be in the same order.  Unlike with normal floating point
 //  comparisons, Tree(NAN) == Tree(NAN).
bool operator == (const Tree& a, const Tree& b);
 // Theoretically we could add < and friends, but it's a pain to program
static bool operator != (const Tree& a, const Tree& b) { return !(a == b); }

namespace X {
    struct WrongForm : LogicError {
        Form form;
        Tree tree;
        WrongForm (Form f, Tree t) : form(f), tree(t) { }
    };
    struct CantRepresent : LogicError {
        String type_name;
        Tree tree;
        CantRepresent (const char* n, Tree t) : type_name(n), tree(t) { }
    };
}

namespace in {
    TreeData* TreeData_bool (bool);
}
template <class T, std::enable_if_t<std::is_same_v<std::decay_t<T>, bool>, bool>>
Tree::Tree (T v) : Tree(in::TreeData_bool(v)) { }

}  // namespace hacc
