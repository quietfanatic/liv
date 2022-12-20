 // This defines the main Tree datatype which represents an AYU structure.
 // Trees are immutable and reference-counted, so copying is cheap, but they
 // can't be accessed on multiple threads at a time.

#pragma once

#include <exception>
#include "internal/common-internal.h"

namespace ayu {

 // For unambiguity, types of trees are called forms.
enum TreeForm : uint8 {
    UNDEFINED = 0,
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
Str form_name (TreeForm);

 // Options that control how a Tree is printed.  These do not have any effect on
 // the semantics of the Tree, and they do not affect subtrees.
using TreeFlags = uint16;
enum : TreeFlags {
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

struct Tree {
    const TreeForm form;
    const uint8 rep;
    TreeFlags flags;
    const union {
        usize as_usize;
        int64 as_int64;
        double as_double;
        const in::RefCounted* as_ptr;
    } data;

    bool has_value () const { return form != UNDEFINED; }

     // Default construction.  The only valid operation on an UNDEFINED tree is
     // has_value().
    constexpr Tree () :
        form(UNDEFINED), rep(0), flags(0), data{.as_int64 = 0}
    { }
    constexpr Tree (Tree&& o) :
        form(o.form), rep(o.rep), flags(o.flags), data(o.data)
    {
        const_cast<TreeForm&>(o.form) = UNDEFINED;
        const_cast<uint8&>(o.rep) = 0;
        o.flags = 0;
        const_cast<int64&>(o.data.as_int64) = 0;
    }
    Tree (const Tree&);
    ~Tree ();

    Tree& operator = (const Tree& o) {
        this->~Tree();
        new (this) Tree(o);
        return *this;
    }
    Tree& operator = (Tree&& o) {
        this->~Tree();
        new (this) Tree(std::move(o));
        return *this;
    }

    explicit Tree (Null v, TreeFlags flags = 0);

     // Disable implicit coercion of the argument to bool
    struct ExplicitBool { bool v; };
    explicit Tree (ExplicitBool, TreeFlags flags);
    template <class T> requires (std::is_same_v<std::decay_t<T>, bool>)
    explicit Tree (T v, TreeFlags flags = 0) : Tree(ExplicitBool{v}, flags) { }

     // plain (not signed or unsigned) chars are represented as strings
    explicit Tree (char v, TreeFlags flags = 0) : Tree(String(1,v), flags) { }
    explicit Tree (int8 v, TreeFlags flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (uint8 v, TreeFlags flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (int16 v, TreeFlags flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (uint16 v, TreeFlags flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (int32 v, TreeFlags flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (uint32 v, TreeFlags flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (int64 v, TreeFlags flags = 0);
    explicit Tree (uint64 v, TreeFlags flags = 0) : Tree(int64(v), flags) { }
    explicit Tree (float v, TreeFlags flags = 0) : Tree(double(v), flags) { }
    explicit Tree (double v, TreeFlags flags = 0);
    explicit Tree (Str v, TreeFlags flags = 0) : Tree(String(v), flags) { }
    explicit Tree (String&& v, TreeFlags flags = 0);
    explicit Tree (Str16 v, TreeFlags flags = 0) : Tree(String16(v), flags) { }
    explicit Tree (String16&& v, TreeFlags flags = 0); // Converts to UTF8 internally
    explicit Tree (const char* v, TreeFlags flags = 0) : Tree(String(v), flags) { }
    explicit Tree (Array v, TreeFlags flags = 0);
    explicit Tree (Object v, TreeFlags flags = 0);
    explicit Tree (std::exception_ptr p, TreeFlags flags = 0);

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
    const Tree* attr (Str key) const;
     // Returns null if the invocant is not an ARRAY or does not have an
     // element at the given index.
    const Tree* elem (usize index) const;

     // Throws if the tree is not an object or doesn't have that attribute.
    const Tree& operator[] (Str key) const;
     // Throws if the tree is not an array or the index is out of bounds.
    const Tree& operator[] (usize index) const;
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
    TreeForm form;
    Tree tree;
};
 // Tried to extract a number from a tree, but the tree's number won't fit
 // into the requested type.
struct CantRepresent : TreeError {
    String type_name;
    Tree tree;
};

}  // namespace ayu
