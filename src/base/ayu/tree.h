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
     // For ARRAY or OBJECT: When pretty-printing, print this item compactly,
     // all on one line (unless one of its children is expanded).
     // For STRING: When printing in non-JSON mode, encode newlines and tabs as
     // \n and \t.
    PREFER_COMPACT = 1 << 1,
     // For ARRAY or OBJECT: When pretty-printing, print fully expanded with one
     // element/attribute per line.
     // For STRING: When printing in non-JSON mode, print newlines and tabs
     // as-is without escaping them.
     // If neither PREFER_EXPANDED nor PREFER_COMPACT is set, the printer will
     // use some heuristics to decide which way to print it.  If both are set,
     // which one takes priority is unspecified.
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
        const char* as_char_ptr;
        const Tree* as_array_ptr;
        const TreePair* as_object_ptr;
        const std::exception_ptr* as_error_ptr;
    } data;

    constexpr bool has_value () const { return form != UNDEFINED; }

     // Default construction.  The only valid operation on an UNDEFINED tree is
     // has_value().
    constexpr Tree ();
     // Move construction.
    constexpr Tree (Tree&& o);
     // Copy construction.  May twiddle reference counts.
    constexpr Tree (const Tree&);
     // Destructor.
    constexpr ~Tree ();
     // Assignment boilerplate
    constexpr Tree& operator = (Tree&& o) {
        if (this == &o) [[unlikely]] return *this;
        this->~Tree();
        return *new (this) Tree(move(o));
    }
    constexpr Tree& operator = (const Tree& o) {
        if (this == &o) [[unlikely]] return *this;
        this->~Tree();
        return *new (this) Tree(o);
    }

    ///// CONVERSION TO TREE
    explicit constexpr Tree (Null);
     // Disable implicit coercion of the argument to bool
    template <class T> requires (std::is_same_v<T, bool>)
    explicit constexpr Tree (T);
     // Templatize this instead of providing an overload for each int type, to
     // shorten error messages about "no candidate found".
    template <class T> requires (
         // ACTUAL integer, not bool or char
        std::is_integral_v<T> &&
        !std::is_same_v<T, bool> && !std::is_same_v<T, char>
    )
    explicit constexpr Tree (T);
     // May as well do this too
    template <class T> requires (std::is_floating_point_v<T>)
    explicit constexpr Tree (T);

     // plain (not signed or unsigned) chars are represented as strings
     // This is not optimal but who serializes individual 8-bit code units
    explicit Tree (char v) : Tree(SharedString(1,v)) { }
    explicit constexpr Tree (AnyString);
    explicit Tree (Str16); // Converts to UTF8

    explicit constexpr Tree (TreeArray);
    explicit constexpr Tree (TreeObject);
    explicit Tree (std::exception_ptr);

    ///// CONVERSION FROM TREE
     // These throw if the tree is not the right form or if
     // the requested type cannot store the value, e.g. try to convert to a
     // uint8 a Tree containing the number 257.
    explicit constexpr operator Null () const;
    explicit constexpr operator bool () const;
    explicit constexpr operator char () const;
    explicit constexpr operator int8 () const;
    explicit constexpr operator uint8 () const;
    explicit constexpr operator int16 () const;
    explicit constexpr operator uint16 () const;
    explicit constexpr operator int32 () const;
    explicit constexpr operator uint32 () const;
    explicit constexpr operator int64 () const;
    explicit constexpr operator uint64 () const;
    explicit constexpr operator float () const { return double(*this); }
    explicit constexpr operator double () const;
     // Warning 1: The returned Str is not NUL-terminated.
     // Warning 2: The Str will be invalidated when this Tree is destructed.
    explicit constexpr operator Str () const;
    explicit constexpr operator AnyString () const&;
    explicit operator AnyString () &&;
    explicit operator UniqueString16 () const;
    explicit constexpr operator TreeArraySlice () const;
    explicit constexpr operator TreeArray () const&;
    explicit operator TreeArray () &&;
    explicit constexpr operator TreeObjectSlice () const;
    explicit constexpr operator TreeObject () const&;
    explicit operator TreeObject () &&;
    explicit operator std::exception_ptr () const;

    ///// CONVENIENCE
     // Returns null if the invocant is not an OBJECT or does not have an
     // attribute with the given key.
    constexpr const Tree* attr (Str key) const;
     // Returns null if the invocant is not an ARRAY or does not have an
     // element at the given index.
    constexpr const Tree* elem (usize index) const;

     // Throws if the tree is not an object or doesn't have that attribute.
    constexpr const Tree& operator[] (Str key) const;
     // Throws if the tree is not an array or the index is out of bounds.
    constexpr const Tree& operator[] (usize index) const;
};
 // Make sure earlier CRef<Tree, 16> alias is correct
static_assert(sizeof(Tree) == sizeof(TreeRef));

 // Test for equality.  Trees of different forms are considered unequal.
 //  - Unlike float and double, Tree(NAN) == Tree(NAN).
 //  - Like float and double, -0.0 == +0.0.
 //  - Objects are equal if they have all the same attributes, but the
 //  attributes don't have to be in the same order.  Note that comparing
 //  TreeObjects or TreeObjectSlices will NOT do an order-independent
 //  comparison, it'll just do ordinary array comparison.
bool operator == (TreeRef a, TreeRef b);

struct TreeError : Error { };
 // Tried to treat a tree as though it's a form which it's not.
struct WrongForm : TreeError {
    TreeForm form;
    Tree tree;
};
 // Tried to extract a number from a tree, but the tree's number won't fit
 // into the requested type.
struct CantRepresent : TreeError {
    AnyString type_name;
    Tree tree;
};

}  // namespace ayu

#include "internal/tree-internal.h"
