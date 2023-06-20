 // This defines the main Tree datatype which represents an AYU structure.
 // Trees are immutable and reference-counted, so copying is cheap, but they
 // can't be accessed on multiple threads at a time.

#pragma once

#include <exception>
#include "internal/common-internal.h"
#include "../uni/copy-ref.h"

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
    const int8 rep;
     // Only the flags can be modified after construction.
    TreeFlags flags;
     // Only defined for certain forms.
    const uint32 length;
    const union {
        bool as_bool;
        int64 as_int64;
        double as_double;
        char as_chars [8];
        const char* as_char_ptr;
        const Tree* as_array_ptr;
        const TreePair* as_object_ptr;
        std::exception_ptr* as_error_ptr;
    } data;

    bool has_value () const { return form != UNDEFINED; }

     // Default construction.  The only valid operation on an UNDEFINED tree is
     // has_value().
    constexpr Tree () :
        form(UNDEFINED), rep(0), flags(0), length(0), data{.as_int64 = 0}
    { }
     // Move construction.
    constexpr Tree (Tree&& o) :
        form(o.form), rep(o.rep), flags(o.flags), length(o.length), data(o.data)
    {
        const_cast<TreeForm&>(o.form) = UNDEFINED;
        const_cast<int8&>(o.rep) = 0;
         // TODO: These are not needed
        o.flags = 0;
        const_cast<uint32&>(o.length) = 0;
        const_cast<int64&>(o.data.as_int64) = 0;
    }
     // Copy construction.  May twiddle reference counts.
    Tree (const Tree&);
     // Destructor.
    ~Tree ();

    Tree& operator = (const Tree& o) {
        this->~Tree();
        return *new (this) Tree(o);
    }
    Tree& operator = (Tree&& o) {
        this->~Tree();
        return *new (this) Tree(std::move(o));
    }

    explicit Tree (Null);

     // Disable implicit coercion of the argument to bool
    struct ExplicitBool { bool v; };
    explicit Tree (ExplicitBool);
    template <class T> requires (std::is_same_v<std::decay_t<T>, bool>)
    explicit Tree (T v) : Tree(ExplicitBool{v}) { }

     // plain (not signed or unsigned) chars are represented as strings
    explicit Tree (char v) : Tree(std::string(1,v)) { }
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
    explicit Tree (Str v);
    template <class T> requires (requires (T v) { Str(v); })
    explicit Tree (T v) : Tree(Str(v)) { } // TEMP
    explicit Tree (OldStr16 v); // Converts to UTF8
    explicit Tree (TreeArray v);
    explicit Tree (TreeObject v);
    explicit Tree (std::exception_ptr p);

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
     // Warning 1: The returned OldStr() is not NUL-terminated.
     // Warning 2: If you get a OldStr() from a Tree or a TreeRef, that OldStr() will
     // only be valid while that Tree or TreeRef exists (even if the original
     // Tree exists).  If you need to access the string later, copy the whole
     // Tree (it's cheap).
    explicit operator OldStr () const;
    explicit operator AnyString () const { return AnyString(OldStr(*this)); } // TEMP
    explicit operator std::string () const;  // Does a copy.
    explicit operator std::u16string () const;
    explicit operator TreeArraySlice () const;
    explicit operator TreeArray () const;
    explicit operator TreeObjectSlice () const;
    explicit operator TreeObject () const;

     // Returns null if the invocant is not an OBJECT or does not have an
     // attribute with the given key.
    const Tree* attr (OldStr key) const;
     // Returns null if the invocant is not an ARRAY or does not have an
     // element at the given index.
    const Tree* elem (usize index) const;

     // Throws if the tree is not an object or doesn't have that attribute.
    const Tree& operator[] (OldStr key) const;
     // Throws if the tree is not an array or the index is out of bounds.
    const Tree& operator[] (usize index) const;
};
 // Make sure earlier CRef<Tree, 16> alias is correct
static_assert(sizeof(Tree) == 16);

 // Test for equality.  Trees of different forms are considered unequal.
 // Objects are equal if all their attributes are the same; the attributes
 // don't have to be in the same order.  Unlike with normal floating point
 // comparisons, Tree(NAN) == Tree(NAN).  -0.0 and +0.0 are considered equal.
bool operator == (TreeRef a, TreeRef b);
 // Theoretically we could add < and friends, but it's a pain to program.

 // If we're gonna start using Trees as strings, we'll want this
bool operator == (TreeRef a, OldStr b);

struct TreeError : Error { };
 // Tried to treat a tree as though it's a form which it's not.
struct WrongForm : TreeError {
    TreeForm form;
    Tree tree;
};
 // Tried to extract a number from a tree, but the tree's number won't fit
 // into the requested type.
struct CantRepresent : TreeError {
    std::string type_name;
    Tree tree;
};

}  // namespace ayu
