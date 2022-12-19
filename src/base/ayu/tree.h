 // This defines the main Tree datatype which represents an AYU structure.
 // Trees are immutable and reference-counted, so copying is cheap, but they
 // can't be accessed on multiple threads at a time.

#pragma once

#include "internal/common-internal.h"

namespace ayu {

 // For unambiguity, types of trees are called forms.
enum Form {
    NULLFORM,
    BOOL,
    NUMBER,
    STRING,
    ARRAY,
    OBJECT,
     // A form that carries a stored exception, used for error reporting.
     // If you try to do anything with it, it will probably throw its contents.
    ERROR
};
 // Readable name of a form in lowercase.
Str form_name (Form);

 // Options that control how a Tree is printed.  These do not have any effect on
 // the semantics of the Tree, and they do not affect subtrees.
enum TreeFlags : uint16 {
     // For NUMBER: Print the number as hexadecimal.
    PREFER_HEX = 1 << 0,
     // For ARRAY or OBJECT: When pretty-printing, prefer printing this item
     // compactly.
     // For STRING: When printing in non-JSON mode, encode newlines and tabs as
     // \n and \t.
    PREFER_COMPACT = 1 << 1,
     // For ARRAY or OBJECT: When pretty-printing, prefer printing this item
     // fully expanded with one element/attribute per line.
     // For STRING: When printing in non-JSON mode, print newlines and tabs
     // as-is without escaping them.
    PREFER_EXPANDED = 1 << 2,

    VALID_TREE_FLAG_BITS = PREFER_HEX | PREFER_COMPACT | PREFER_EXPANDED
};
constexpr TreeFlags operator | (TreeFlags a, TreeFlags b) {
    return TreeFlags(uint16(a) | uint16(b));
}

struct Tree {
    in::RCP<in::TreeData, in::delete_TreeData> data;
    constexpr explicit Tree (in::TreeData* data = null) : data(data) { }
    bool has_value () const { return !!data; }

    explicit Tree (Null v, uint16 flags = 0);
     // Disable implicit coercion of the argument to bool
    template <class T> requires (std::is_same_v<std::decay_t<T>, bool>)
    explicit Tree (T v, uint16 flags = 0);
     // plain (not signed or unsigned) chars are represented as strings
    explicit Tree (char v, uint16 flags = 0) : Tree(String(1,v), flags) { }
    explicit Tree (int8 v, uint16 flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (uint8 v, uint16 flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (int16 v, uint16 flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (uint16 v, uint16 flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (int32 v, uint16 flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (uint32 v, uint16 flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (int64 v, uint16 flags = 0);
    explicit Tree (uint64 v, uint16 flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (float v, uint16 flags = 0) : Tree(double(v), flags) { }
    explicit Tree (double v, uint16 flags = 0);
    explicit Tree (Str v, uint16 flags = 0) : Tree(String(v), flags) { }
    explicit Tree (String&& v, uint16 flags = 0);
    explicit Tree (Str16 v, uint16 flags = 0) : Tree(String16(v), flags) { }
    explicit Tree (String16&& v, uint16 flags = 0); // Converts to UTF8 internally
    explicit Tree (const char* v, uint16 flags = 0) : Tree(String(v), flags) { }
    explicit Tree (const Array& v, uint16 flags = 0);
    explicit Tree (Array&& v, uint16 flags = 0);
    explicit Tree (const Object& v, uint16 flags = 0);
    explicit Tree (Object&& v, uint16 flags = 0);

    Form form () const;
     // Get flags for printing this tree.  This is NOT guaranteed to be equal to
     // the flags that were passed in (in particular, flags that don't apply to
     // the given Tree may or may not be omitted).
    uint16 flags () const;

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
    explicit operator String16 () const;
    explicit operator const Array& () const;
    explicit operator const Object& () const;

     // Returns null if the invocant is not an OBJECT or does not have an
     // attribute with the given key.
    Tree* attr (Str key) const;
     // Returns null if the invocant is not an ARRAY or does not have an
     // element at the given index.
    Tree* elem (usize index) const;

     // Throws if the tree is not an object or doesn't have that attribute.
    Tree operator[] (Str key) const;
     // Throws if the tree is not an array or the index is out of bounds.
    Tree operator[] (usize index) const;
};

 // Test for equality.  Trees of different forms are considered unequal.
 // Objects are equal if all their attributes are the same; the attributes
 // don't have to be in the same order.  Unlike with normal floating point
 // comparisons, Tree(NAN) == Tree(NAN).  -0.0 and +0.0 are considered equal.
bool operator == (const Tree& a, const Tree& b);
 // Theoretically we could add < and friends, but it's a pain to program.
inline bool operator != (const Tree& a, const Tree& b) { return !(a == b); }

struct TreeError : Error { };
 // Tried to treat a tree as though it's a form which it's not.
struct WrongForm : TreeError {
    Form form;
    Tree tree;
};
 // Tried to extract a number from a tree, but the tree's number won't fit
 // into the requested type.
struct CantRepresent : TreeError {
    String type_name;
    Tree tree;
};

namespace in {
    TreeData* TreeData_bool (bool, uint16 flags);
}
template <class T> requires (std::is_same_v<std::decay_t<T>, bool>)
Tree::Tree (T v, uint16 flags) : Tree(in::TreeData_bool(v, flags)) { }

}  // namespace ayu
