 // Arrays that can be shared (ref-counted) or static
 //
 // This header provides a constellation of array and string classes that share
 // a common interface and differ by ownership model.  They're largely
 // compatible with STL containers, but not quite a drop-in replacement.
 //
 // COPY-ON-WRITE
 // The AnyArray and AnyString classes have copy-on-write behavior when you try
 // to modify them.  By default simple access operations like begin(), end(),
 // operator[], and at() do not trigger a copy-on-write and instead return const
 // references.  To trigger a copy-on-write use mut_begin(), mut_end(),
 // mut_get(), and mut_at().  For simplicity, AnyArray and AnyString can only be
 // used with element types that have a copy constructor.  To work with move-
 // only element types, use UniqueArray.
 //
 // STATIC STRING OPTIMIZATION
 // Not to be confused with Small String Optimization.  AnyArray and AnyString
 // can be refer to a static string (a string which will stay alive for the
 // duration of the program, or at least the foreseeable future).  This allows
 // them to be created and passed around with no allocation cost.
 //
 // THREAD-SAFETY
 // SharedArray and AnyArray use reference counting which is not threadsafe.
 // To pass arrays between threads you should use UniqueArray.
 //
 // EXCEPTION-SAFETY
 // None of these classes generate their own exceptions.  If an out-of-bounds
 // index or a too-large-for-capacity problem occurs, the program will be
 // terminated.  If an element type throws an exception in its default
 // constructor, copy constructor, or copy assignment operator, the array method
 // provides a mostly-strong exception guarantee (except for multi-element
 // insert): All semantic state will be rewound to before the method was called,
 // but non-semantic state (such as capacity and whether a buffer is shared) may
 // be altered.  However, if an element type throws an exception in its move
 // constructor, move assignment constructor, or destructor, undefined behavior
 // will occur (hopefully after a debug-only assertion).
#pragma once

#include <filesystem> // for conversion to path
#include <functional> // std::hash
#include <iterator>
#include "array-implementations.h"
#include "common.h"
#include "requirements.h"

namespace uni {
inline namespace arrays {

///// THIS HEADER PROVIDES

 // The base interface for all array classes.
template <ArrayClass ac, class T>
struct ArrayInterface;

 // A generic dynamically-sized array class which can either own shared
 // (refcounted) data or reference static data.  Has semi-implicit
 // copy-on-write behavior (at no cost if you don't use it).
template <class T>
using AnyArray = ArrayInterface<ArrayClass::AnyA, T>;

 // An array that can only reference shared data.  There isn't much reason to
 // use this instead of AnyArray, but it's here as an intermediate between AnyArray
 // and UniqueArray.  This should probably be renamed because its name suggests
 // it might be have shared mutability, but it does not.
template <class T>
using SharedArray = ArrayInterface<ArrayClass::SharedA, T>;

 // An array that guarantees unique ownership, allowing mutation without
 // copy-on-write.  This has the same role as std::vector.
template <class T>
using UniqueArray = ArrayInterface<ArrayClass::UniqueA, T>;

 // An array that can only reference static data (or at least data that outlives
 // it).  The data cannot be modified.  The difference between this and Slice is
 // that an AnyArray can be constructed from a StaticArray without allocating a new
 // buffer.
template <class T>
using StaticArray = ArrayInterface<ArrayClass::StaticA, T>;

 // A non-owning view of contiguous elements.  This has the same role as
 // std::span (but without fixed extents).
template <class T>
using Slice = ArrayInterface<ArrayClass::SliceA, T>;

 // The string types are almost exactly the same as the equivalent array types.
 // The only differences are that they can be constructed from a const T*, which
 // is taken to be a C-style NUL-terminated string, and when constructing from a
 // C array they will stop at a NUL terminator (the first element that boolifies
 // to false).  Note that by default these strings are not NUL-terminated.  To
 // get a NUL-terminated string out, either explicitly NUL-terminate them or use
 // c_str().
template <class T>
using GenericAnyString = ArrayInterface<ArrayClass::AnyS, T>;
template <class T>
using GenericSharedString = ArrayInterface<ArrayClass::SharedS, T>;
template <class T>
using GenericUniqueString = ArrayInterface<ArrayClass::UniqueS, T>;
template <class T>
using GenericStaticString = ArrayInterface<ArrayClass::StaticS, T>;
template <class T>
using GenericStr = ArrayInterface<ArrayClass::SliceS, T>;

using AnyString = GenericAnyString<char>;
using SharedString = GenericSharedString<char>;
using UniqueString = GenericUniqueString<char>;
using StaticString = GenericStaticString<char>;
using Str = GenericStr<char>;

using AnyString16 = GenericAnyString<char16>;
using SharedString16 = GenericSharedString<char16>;
using UniqueString16 = GenericUniqueString<char16>;
using StaticString16 = GenericStaticString<char16>;
using Str16 = GenericStr<char16>;

using AnyString32 = GenericAnyString<char32>;
using SharedString32 = GenericSharedString<char32>;
using UniqueString32 = GenericUniqueString<char32>;
using StaticString32 = GenericStaticString<char32>;
using Str32 = GenericStr<char32>;

///// ARRAYLIKE CONCEPTS
 // A general concept for array-like types.
template <class A>
concept ArrayLike = requires (A a) { *a.data(); usize(a.size()); };
 // An array-like type that is NOT one of the classes defined here.
template <class A>
concept OtherArrayLike = ArrayLike<A> && !requires { A::is_ArrayInterface; };
 // Array-like for a specific element type.
template <class A, class T>
concept ArrayLikeFor = ArrayLike<A> && std::is_same_v<
    std::remove_cvref_t<decltype(*std::declval<A>().data())>, T
>;
 // Foreign array-like class with specific element type.
template <class A, class T>
concept OtherArrayLikeFor = ArrayLikeFor<A, T> &&
    !requires { A::is_ArrayInterface; };

///// UTILITY CONCEPTS AND STUFF
 // For these concepts, we are deliberately not constraining return types.  Our
 // policy for concepts is to use them for overload resolution, not for semantic
 // guarantees.  And since the requires() clause of a function is part of its
 // interface, we want to keep it as simple as possible.  The STL iterator
 // concepts library is WAY too complicated for this purpose.

 // An ArrayIterator is just anything that can be dereferenced and incremented.
template <class I>
concept ArrayIterator = requires (I i) { *i; ++i; };
 // An ArrayIteratorFor<T> is an ArrayIterator that when dereferenced provides a
 // T.
template <class I, class T>
concept ArrayIteratorFor = ArrayIterator<I> && std::is_same_v<
    std::remove_cvref_t<decltype(*std::declval<I>())>,
    std::remove_cv_t<T>
>;
 // An ArraySentinelFor<Begin> is an iterator that can be compared to Begin with
 // !=.  That is its only requirement; it doesn't have to be dereferencable.
template <class End, class Begin>
concept ArraySentinelFor = requires (Begin b, End e) { b != e; };
 // An ArrayContiguousIterator is one that guarantees that the elements are
 // contiguous in memory like a C array.
 // (We're reluctantly delegating to STL for contiguousity because it cannot be
 // verified at compile time, so the STL just explicitly specifies it for
 // iterators that model it.)
template <class I>
concept ArrayContiguousIterator = std::is_base_of_v<
    std::contiguous_iterator_tag,
    typename std::iterator_traits<std::remove_cvref_t<I>>::iterator_concept
>;
 // An ArrayContiguousIteratorFor<T> can be martialled into a T*.
template <class I, class T>
concept ArrayContiguousIteratorFor =
    ArrayContiguousIterator<I> && std::is_same_v<
        std::remove_cvref_t<decltype(*std::declval<I>())>,
        std::remove_cv_t<T>
    >;
 // An ArrayForwardIterator is one that can be copied, meaning that the array
 // can be walked through multiple times.
template <class I>
concept ArrayForwardIterator = ArrayIterator<I> && std::is_copy_constructible_v<I>;

///// ARRAY INTERFACE
// The shared interface for all the array classes

template <ArrayClass ac, class T>
struct ArrayInterface {
    static_assert(
        alignof(T) <= 8, "Arrays with elements that have align > 8 are NYI."
    );
  private:
    ArrayImplementation<ac, T> impl;
    template <ArrayClass, class>
    friend struct ArrayInterface;
    using Self = ArrayInterface<ac, T>;
  public:

    ///// PSEUDO-CONCEPTS
    static constexpr bool is_ArrayInterface = true;
    static constexpr bool is_String = ac == ArrayClass::AnyS ||
                                      ac == ArrayClass::SharedS ||
                                      ac == ArrayClass::UniqueS ||
                                      ac == ArrayClass::StaticS ||
                                      ac == ArrayClass::SliceS;
    static constexpr bool is_Any = ac == ArrayClass::AnyA ||
                                   ac == ArrayClass::AnyS;
    static constexpr bool is_Shared = ac == ArrayClass::SharedA ||
                                      ac == ArrayClass::SharedS;
    static constexpr bool is_Unique = ac == ArrayClass::UniqueA ||
                                      ac == ArrayClass::UniqueS;
    static constexpr bool is_Static = ac == ArrayClass::StaticA ||
                                      ac == ArrayClass::StaticS;
    static constexpr bool is_Slice = ac == ArrayClass::SliceA ||
                                     ac == ArrayClass::SliceS;
    static constexpr bool supports_share = is_Any || is_Shared;
    static constexpr bool supports_owned = supports_share || is_Unique;
    static constexpr bool supports_static = is_Any || is_Static;
    static constexpr bool trivially_copyable = is_Static || is_Slice;

    static_assert(
        !(supports_share && !std::is_copy_constructible_v<T>),
        "Cannot have a copy-on-write array/string with a non-copyable element type."
    );

    ///// TYPEDEFS
    using value_type = T;
    using size_type = usize;
    using difference_type = isize;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = std::conditional_t<is_Unique, T*, const T*>;
    using const_iterator = const T*;
    using mut_iterator = std::conditional_t<supports_owned, T*, void>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const T*>;
    using mut_reverse_iterator =
        std::conditional_t<supports_owned, std::reverse_iterator<T*>, void>;

    using SelfSlice = std::conditional_t<
        is_String, ArrayInterface<ArrayClass::SliceS, T>, Slice<T>
    >;

    ///// CONSTRUCTION
     // Default construct, makes an empty array.
    ALWAYS_INLINE constexpr
    ArrayInterface () : impl{} { }

     // Move construct.
    constexpr
    ArrayInterface (ArrayInterface&& o) requires (!trivially_copyable) {
        impl = o.impl;
        o.impl = {};
    }
     // Move conversion.  Tries to make the moved-to array have the same
     // ownership mode as the moved-from array, and if that isn't supported,
     // copies the buffer.  Although move conversion will never fail for
     // copyable element types, it's disabled for StaticArray and Slice, and the
     // copy constructor from AnyArray to StaticArray can fail.
    template <ArrayClass ac2> constexpr
    ArrayInterface (ArrayInterface<ac2, T>&& o) requires (
        !trivially_copyable && !o.trivially_copyable
    ) {
        if (supports_share || o.unique()) {
            set_as_owned(o.impl.data, o.size());
            o.impl = {};
        }
        else if constexpr (std::is_copy_constructible_v<T>) {
            set_as_copy(o.impl.data, o.size());
        }
        else never();
    }

     // Copy construct.  Always copies the buffer for UniqueArray, never copies
     // the buffer for other array classes.
    constexpr
    ArrayInterface (const ArrayInterface& o) requires (!trivially_copyable) {
        if constexpr (is_Unique) {
            set_as_copy(o.impl.data, o.size());
        }
        else if constexpr (supports_share) {
            impl = o.impl;
            add_ref();
        }
        else {
            set_as_unowned(o.impl.data, o.size());
        }
    }
     // Copy constructor is defaulted for StaticArray and Slice so that they can
     // have the is_trivially_copy_constructible trait.
    constexpr
    ArrayInterface (const ArrayInterface&) requires (trivially_copyable)
        = default;

     // Copy convert.  Tries to make this array have the same ownership mode as
     // the passed array, and if that isn't supported, tries to copy, and if
     // that isn't supported, fails (usually at compile-time).
     //   - Explicit for converting from AnyArray to StaticArray, since that can
     //     fail at run time if the passed array isn't static.
     //   - You are not allowed to convert from a Slice to a StaticArray at
     //     runtime, because there's no guarantee that the data actually is
     //     static.
    template <ArrayClass ac2> constexpr
    explicit(is_Static && ArrayInterface<ac2, T>::is_Any)
    ArrayInterface (const ArrayInterface<ac2, T>& o) requires (
        supports_owned || !o.is_Slice
    ) {
        if constexpr (o.is_Slice) {
             // Always copy from Slice because we don't know where it came from
             // or where it's going.
            set_as_copy(o.impl.data, o.size());
        }
        else if constexpr (is_Slice) {
            set_as_unowned(o.impl.data, o.size());
        }
        else if constexpr (is_Unique || o.is_Unique) {
            set_as_copy(o.impl.data, o.size());
        }
        else if (o.owned()) {
            if constexpr (supports_share && o.supports_share) {
                set_as_owned(o.impl.data, o.size());
                add_ref();
            }
            else {
                 // We're a StaticArray but we've been given non-static data.
                 // There's nothing we can do.
                require(false);
            }
        }
        else if constexpr (supports_static) {
            set_as_unowned(o.impl.data, o.size());
        }
        else {
            set_as_copy(o.impl.data, o.size());
        }
    }

     // However, at compile-time you can implicity construct a StaticArray from
     // a Slice, since it's nearly guaranteed to be static data (and if it
     // isn't, you'll get a compile error).
    template <ArrayClass ac2> consteval
    ArrayInterface (const ArrayInterface<ac2, T>& o) requires (
        is_Static && o.is_Slice
    ) {
        set_as_unowned(o.impl.data, o.size());
    }

     // Copy construction from other array-like types.  Explicit except for
     // Slice/Str.
    template <OtherArrayLike O> constexpr
    explicit(!(is_Slice && ArrayLikeFor<O, T>))
    ArrayInterface (const O& o) requires (!is_Static) {
        if constexpr (is_Slice) {
            static_assert(ArrayLikeFor<O, T>,
                "Cannot construct Slice from array-like type if its element "
                "type does not match exactly."
            );
            static_assert(ArrayContiguousIterator<decltype(o.data())>,
                "Cannot construct Slice from array-like type if its data() "
                "returns a non-contiguous iterator."
            );
            set_as_unowned(o.data(), o.size());
        }
        else {
            set_as_copy(o.data(), o.size());
        }
    }
     // Actually, allow implicit construction of StaticArray from foreign
     // array-like types, but only at compile-time.  If the data isn't actually
     // static, a well-defined compile-time error will occur.
    template <OtherArrayLikeFor<T> O> consteval
    ArrayInterface (const O& o) requires (is_Static) {
        static_assert(ArrayContiguousIterator<decltype(o.data())>,
            "Cannot construct static array from array-like type if its data() "
            "returns a non-contiguous iterator."
        );
        set_as_unowned(o.data(), o.size());
    }
     // SPECIAL CASE allow construcing a Str from a char8 OtherArrayLike
    template <OtherArrayLikeFor<char8_t> O> constexpr explicit
    ArrayInterface (const O& o) requires (
        is_String && is_Slice && std::is_same_v<T, char>
    ) {
        static_assert(ArrayContiguousIterator<decltype(o.data())>,
            "Cannot construct Slice from array-like type if its data() "
            "returns a non-contiguous iterator."
        );
        set_as_unowned(reinterpret_cast<const char*>(o.data()), o.size());
    }

     // Constructing from const T* is only allowed for String classes.  It will
     // take elements until the first one that boolifies to false.
    constexpr
    ArrayInterface (const T* p) requires (!is_Static && is_String) {
        expect(p);
        usize s = 0;
        while (p[s]) ++s;
        if constexpr (is_Slice) {
            set_as_unowned(p, s);
        }
        else {
            set_as_copy(p, s);
        }
    }
     // As usual, StaticString can only be implicitly constructed at
     // compile-time.
    consteval
    ArrayInterface (const T* p) requires (is_Static && is_String) {
        expect(p);
        usize s = 0;
        while (p[s]) ++s;
        set_as_unowned(p, s);
    }

     // Constructing from C array is different for Array and String classes.
     // Array classes take the whole array, but String classes only take up to
     // (and not including) the first element that's equal to 0.
     //
     // In addition, for constructing from a C array, AnyString (and AnyArray)
     // behaves like StaticString, not like SharedString, in that it borrows
     // data at compile-time and isn't allowed at run-time.  This is a bit of a
     // compromise to allow string literals to be assigned to AnyStrings without
     // any runtime performance cost.  If you really need to assign a character
     // array to an AnyString at runtime, you must explicitly choose between the
     // Copy and Static named constructors.
    template <class T2, usize len> constexpr
    explicit(!std::is_same_v<T2, T> || !is_Slice)
    ArrayInterface (const T2(& o )[len]) requires (!supports_static) {
        usize s;
        if constexpr (is_String) {
            usize s = 0;
            while (s < len && o[s]) ++s;
        }
        else s = len;
        if constexpr (is_Slice) {
            static_assert(std::is_same_v<T2, T>,
                "Cannot construct Slice from C array if its element type does"
                "not match exactly."
            );
            set_as_unowned(o, s);
        }
        else {
            set_as_copy(o, s);
        }
    }
    template <usize len> consteval
    ArrayInterface (const T(& o )[len]) requires (supports_static) {
        usize s;
        if constexpr (is_String) {
            usize s = 0;
            while (s < len && o[s]) ++s;
        }
        else s = len;
        set_as_unowned(o, s);
    }

     // Construct from a pointer(-like iterator) and a size
    template <ArrayIterator Ptr> explicit constexpr
    ArrayInterface (Ptr p, usize s) requires (!is_Static) {
        if constexpr (is_Slice) {
            static_assert(ArrayIteratorFor<Ptr, T>,
                "Cannot borrow Slice from iterator with non-exact element type."
            );
            static_assert(ArrayContiguousIterator<Ptr>,
                "Cannot borrow Slice from non-contiguous iterator."
            );
            set_as_unowned(std::to_address(move(p)), s);
        }
        else {
            set_as_copy(move(p), s);
        }
    }
    template <ArrayIterator Ptr> explicit consteval
    ArrayInterface (Ptr p, usize s) requires (is_Static) {
        static_assert(ArrayIteratorFor<Ptr, T>,
            "Cannot construct static array from iterator with non-exact "
            "element type."
        );
        static_assert(ArrayContiguousIterator<Ptr>,
            "Cannot construct static array from non-contiguous iterator."
        );
        set_as_unowned(std::to_address(move(p)), s);
    }

     // Construct from a pair of iterators.
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End>
    explicit constexpr
    ArrayInterface (Begin b, End e) requires (!is_Static) {
        if constexpr (is_Slice) {
            static_assert(ArrayIteratorFor<Begin, T>,
                "Cannot borrow Slice from iterator with non-exact element type."
            );
            static_assert(ArrayContiguousIterator<Begin>,
                "Cannot borrow Slice from non-contiguous iterator."
            );
            static_assert(requires { usize(e - b); },
                "Cannot borrow Slice from iterator pair that doesn't support "
                "subtraction to get size."
            );
            set_as_unowned(std::to_address(move(b)), usize(e - b));
        }
        else {
            set_as_copy(move(b), move(e));
        }
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End>
    explicit consteval
    ArrayInterface (Begin b, End e) requires (is_Static) {
        static_assert(ArrayIteratorFor<Begin, T>,
            "Cannot construct static array from iterator with non-exact "
            "element type."
        );
        static_assert(ArrayContiguousIterator<Begin>,
            "Cannot construct static array from non-contiguous iterator."
        );
        static_assert(requires { usize(e - b); },
            "Cannot construct static array from iterator pair that doesn't "
            "support subtraction to get size."
        );
        set_as_unowned(std::to_address(b), usize(e - b));
    }

     // Construct an array with a number of default-constructed elements.
    explicit
    ArrayInterface (usize s) requires (
        supports_owned && std::is_default_constructible_v<T>
    ) {
        if (!s) {
            impl = {}; return;
        }
        T* dat = allocate_owned(s);
        usize i = 0;
        try {
            for (; i < s; ++i) {
                new ((void*)&dat[i]) T();
            }
        }
        catch (...) {
            while (i > 0) {
                impl.data[--i].~T();
            }
            deallocate_owned(dat);
            throw;
        }
        set_as_unique(dat, s);
    }

     // This constructor constructs a repeating sequence of one element.  It's
     // only available for owned classes.
    explicit
    ArrayInterface (usize s, const T& v) requires (
        supports_owned && std::is_copy_constructible_v<T>
    ) {
        if (!s) {
            impl = {}; return;
        }
        T* dat = allocate_owned(s);
        usize i = 0;
        try {
            for (; i < s; ++i) {
                new ((void*)&dat[i]) T(v);
            }
        }
        catch (...) {
            while (i > 0) {
                impl.data[--i].~T();
            }
            deallocate_owned(dat);
            throw;
        }
        set_as_unique(dat, s);
    }

     // Finally, std::initializer_list
    ArrayInterface (std::initializer_list<T> l) requires (supports_owned || is_Slice) {
        if constexpr (is_Slice) {
            set_as_unowned(std::data(l), std::size(l));
        }
        else set_as_copy(std::data(l), std::size(l));
    }

    ///// NAMED CONSTRUCTORS
    // These allow you to explicitly specify a construction method.  DEPRECATED!
    // TODO: remove these

     // Explicitly share a shared array.  Will fail at runtime if given an AnyArray
     // with static data.
    static
    Self Share (const AnyArray<T>& o) requires (supports_share) {
        require(o.owned() || o.empty());
        return SharedArray<T>(o);
    }
    static
    Self Share (const SharedArray<T>& o) requires (supports_share) {
        return SharedArray<T>(o);
    }

     // Explicitly copy the passed data.
    static
    Self Copy (SelfSlice o) requires (supports_owned) {
        return UniqueArray<T>(o);
    }
    template <ArrayIterator Ptr> static
    Self Copy (Ptr p, usize s) requires (supports_owned) {
        return UniqueArray<T>(move(p), s);
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End> static
    Self Copy (Begin b, End e) requires (supports_owned) {
        return UniqueArray<T>(move(b), move(e));
    }

     // Explicitly reference static data.  Unlike the consteval constructors,
     // these are allowed at run time.  It's the caller's responsibility to make
     // sure that the referenced data outlives both the returned StaticArray AND
     // all AnyArrays and StaticArrays that may be constructed from it.
    static constexpr
    Self Static (SelfSlice o) requires (supports_static) {
        StaticArray<T> r;
        r.set_as_unowned(o.impl.data, o.size());
        return r;
    }
    template <ArrayIterator Ptr> static constexpr
    Self Static (Ptr p, usize s) requires (supports_static) {
        static_assert(ArrayIteratorFor<Ptr, T>,
            "Cannot construct static array from iterator with non-exact "
            "element type."
        );
        static_assert(ArrayContiguousIterator<Ptr>,
            "Cannot construct static array from non-contiguous iterator."
        );
        StaticArray<T> r;
        r.set_as_unowned(std::to_address(move(p)), s);
        return r;
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End> static constexpr
    Self Static (Begin b, End e) requires (supports_static) {
        static_assert(ArrayIteratorFor<Begin, T>,
            "Cannot construct static array from iterator with non-exact "
            "element type."
        );
        static_assert(ArrayContiguousIterator<Begin>,
            "Cannot construct static array from non-contiguous iterator."
        );
        static_assert(requires { usize(e - b); },
            "Cannot construct static array from iterator pair that doesn't "
            "support subtraction to get size."
        );
        StaticArray<T> r;
        r.set_as_unowned(std::to_address(b), usize(e - b));
        return r;
    }

     // Create an array with a given size but uninitialized data.  This is only
     // allowed for element types that are allowed to be uninitialized.
    static
    Self Uninitialized (usize s) requires (is_Unique) {
        static_assert(std::is_trivially_default_constructible_v<T>,
            "Cannot create Uninitialized array with type that isn't trivially "
            "default constructible."
        );
        Self r;
        r.set_as_unique(allocate_owned(s), s);
        return r;
    }

    ///// ASSIGNMENT OPERATORS
    // These only take assignment from the exact same array class, relying on
    // implicit coercion for the others.
    // There is an opportunity for optimization that we aren't taking, that
    // being for copy assignment to unique strings to reuse the allocated buffer
    // instead of making a new one.  Currently I think this is more complicated
    // than it's worth.
    // Also, we should maybe detect when assigning a substr of self to self.

    constexpr
    ArrayInterface& operator= (ArrayInterface&& o) requires (
        !trivially_copyable
    ) {
        if (&o == this) [[unlikely]] return *this;
        this->~ArrayInterface();
        return *new (this) ArrayInterface(move(o));
    }
    constexpr
    ArrayInterface& operator= (const ArrayInterface& o) requires (
        !trivially_copyable
    ) {
        if (&o == this) [[unlikely]] return *this;
        return *this = ArrayInterface(o);
    }
    constexpr
    ArrayInterface& operator= (const ArrayInterface&) requires (
        trivially_copyable
    ) = default;

    ///// COERCION

     // Okay okay
    ALWAYS_INLINE constexpr
    operator std::basic_string_view<T> () const requires (is_String) {
        return std::basic_string_view<T>(impl.data, size());
    }
    ALWAYS_INLINE constexpr
    operator std::basic_string<T> () const requires (is_String) {
        return std::string(impl.data, size());
    }
     // Sigh
    ALWAYS_INLINE constexpr
    operator std::filesystem::path () const requires (
        is_String && sizeof(T) == sizeof(char) && requires (char c, T v) { c = v; }
    ) {
        return std::filesystem::path(
            reinterpret_cast<const char8_t*>(begin()),
            reinterpret_cast<const char8_t*>(end())
        );
    }

    ///// DESTRUCTOR
    constexpr
    ~ArrayInterface () requires (!trivially_copyable) { remove_ref(); }
    constexpr
    ~ArrayInterface () requires (trivially_copyable) = default;

    ///// BYPASSING REFERENCE COUNTING
     // These functions construct or destroy an array without touching the
     // reference counting mechanism, so only use them if you want to manage the
     // reference counts yourself.
    static constexpr
    Self Materialize (T* d, usize s)
        requires (is_Shared || is_Unique)
    {
        Self r;
        r.set_as_owned(d, s);
        return r;
    }
    constexpr
    void dematerialize () { impl = {}; }
    constexpr
    void materialize_size (usize s) { set_size(s); }

    ///// ACCESSORS

     // Gets whether this array is empty (size == 0)
    ALWAYS_INLINE constexpr
    bool empty () const { return size() == 0; }
     // True for non-empty arrays
    ALWAYS_INLINE constexpr
    explicit operator bool () const { return size(); }

     // Get the size of the array in elements
    constexpr
    usize size () const {
        if constexpr (is_Any) {
            return impl.sizex2_with_owned >> 1;
        }
        else return impl.size;
    }

     // Maximum size differs depending on whether this array can be owned or
     // not.  The maximum size for owned arrays is the same on both 32-bit and
     // 64-bit platforms.  If you need to process arrays larger than 2 billion
     // eleents, you're probably already managing your own memory anyway.
    static constexpr usize max_size_ =
        supports_owned ? uint32(-1) >> 1 : usize(-1) >> 1;
    ALWAYS_INLINE constexpr
    usize max_size () const { return max_size_; }

     // Get data pointer.  Always const by default except for UniqueArray, to
     // avoid accidental copy-on-write.
    ALWAYS_INLINE constexpr
    const T* data () const { return impl.data; }
    ALWAYS_INLINE
    T* data () requires (is_Unique) { return impl.data; }
     // Get mutable data pointer, triggering copy-on-write if needed.
    T* mut_data () requires (supports_owned) {
        make_unique();
        return impl.data;
    }

     // Guarantees the return of a NUL-terminated buffer, possibly by attaching
     // a NUL element after the end.  Does not change the size of the array, but
     // may change its capacity.  For StaticArray and Slice, since they can't be
     // mutated, this require()s that the array is explicitly NUL-terminated.
    constexpr
    const T* c_str () requires (requires (T v) { !v; v = T(); }) {
        if (size() > 0 && !impl.data[size()-1]) {
            return impl.data;
        }
        else if constexpr (supports_owned) {
             // We are allowed to use one of two cheats: either allow shared
             // buffers to have different lengths, or allow writing a NUL to the
             // end of a shared buffer without triggering a copy-on-write.  I
             // think it makes sense to do the former for arrays and the latter
             // for strings.
            if (!is_String || capacity() < size() + 1) {
                 // Using just reserve here instead of reserve_plenty, because
                 // it's unlikely that you're going to add more onto the end
                 // after requesting a NUL-terminated string.
                reserve(size() + 1);
            }
            impl.data[size()] = T();
            return impl.data;
        }
        else never();
    }

     // Access individual elements.  at() and mut_at() will terminate if the
     // passed index is out of bounds.  operator[], get(), and mut_get() do not
     // do bounds checks (except on debug builds).  Only the mut_* versions can
     // trigger copy-on-write; the non-mut_* version are always const except for
     // UniqueArray.
     //
     // Note: Using &array[array.size()] to get a pointer off the end of the
     // array is NOT allowed.  Use array.end() instead, or array.data() +
     // array.size().
    constexpr
    const T& at (usize i) const {
        require(i < size());
        return impl.data[i];
    }
    ALWAYS_INLINE
    T& at (usize i) requires (is_Unique) {
        return const_cast<T&>(
            const_cast<const ArrayInterface<ac, T>&>(*this).at(i)
        );
    }
    T& mut_at (usize i) requires (supports_owned) {
        make_unique();
        return const_cast<T&>(at(i));
    }
    constexpr
    const T& operator [] (usize i) const {
        expect(i < size());
        return impl.data[i];
    }
    ALWAYS_INLINE
    T& operator [] (usize i) requires (is_Unique) {
        return const_cast<T&>(
            const_cast<const ArrayInterface<ac, T>&>(*this)[i]
        );
    }
    ALWAYS_INLINE constexpr
    const T& get (usize i) const {
        return (*this)[i];
    }
    ALWAYS_INLINE
    T& get (usize i) requires (is_Unique) {
        return const_cast<T&>(
            const_cast<const ArrayInterface<ac, T>&>(*this)[i]
        );
    }
    T& mut_get (usize i) requires (supports_owned) {
        make_unique();
        return const_cast<T&>((*this)[i]);
    }

    ALWAYS_INLINE constexpr
    const T& front () const { return (*this)[0]; }
    ALWAYS_INLINE constexpr
    T& front () requires (is_Unique) { return (*this)[0]; }
    T& mut_front () requires (supports_owned) {
        make_unique();
        return (*this)[0];
    }

    ALWAYS_INLINE constexpr
    const T& back () const { return (*this)[size() - 1]; }
    ALWAYS_INLINE constexpr
    T& back () requires (is_Unique) { return (*this)[size() - 1]; }
    T& mut_back () requires (supports_owned) {
        make_unique();
        return (*this)[size() - 1];
    }

     // Slice takes two offsets and does not do bounds checking (except in debug
     // builds).  Unlike operator[], the offsets are allowed to be one off the
     // end.
    constexpr
    SelfSlice slice (usize start, usize end) const {
        expect(start <= size() && end <= size());
        return SelfSlice(data() + start, end - start);
    }

     // Substr takes an offset and a length, and caps both to the length of the
     // string.
    constexpr
    SelfSlice substr (usize offset, usize length = usize(-1)) const {
        if (offset >= size()) offset = size();
        if (length > size() - offset) length = size() - offset;
        return SelfSlice(data() + offset, length);
    }

     // Get the current capacity of the owned buffer.  If this array is not
     // owned, returns 0 even if the array is not empty.
    constexpr
    usize capacity () const {
        if (owned()) {
            expect(header().capacity >= min_capacity);
            return header().capacity;
        }
        else return 0;
    }

     // The minimum capacity of a shared buffer (enough elements to fill 24 (on
     // 64-bit) or 16 (on 32-bit) bytes).
    static constexpr usize min_capacity = Self::capacity_for_size(1);

     // Returns if this string is owned (has a shared or unique buffer).  If
     // this returns true, then there is an ArrayOwnedHeader right behind the
     // pointer returned by data().  Returns false for empty arrays.
    constexpr
    bool owned () const {
        if constexpr (is_Any) {
            if (impl.sizex2_with_owned & 1) {
                expect(impl.data);
                return true;
            }
            else return false;
        }
        else if constexpr (supports_owned) {
            if (impl.data) return true;
            else {
                expect(!impl.size);
                return false;
            }
        }
        else return false;
    }

     // Returns if this string is unique (can be moved to a UniqueArray without
     // doing an allocation).  This is not a strict subset of owned(); in
     // particular, it will return true for most empty arrays (capacity == 0).
    constexpr
    bool unique () const {
        if constexpr (is_Unique) return true;
        else if constexpr (supports_owned) {
            if (owned()) {
                return header().ref_count == 0;
            }
            else return !impl.data;
        }
        else return false;
    }

    ///// ITERATORS
    // begin(), end(), and related functions never trigger a copy-on-write, and
    // return const for all but UniqueArray.  The mut_* versions will trigger a
    // copy-on-write.

    ALWAYS_INLINE constexpr
    const T* begin () const { return impl.data; }
    ALWAYS_INLINE constexpr
    const T* cbegin () const { return impl.data; }
    ALWAYS_INLINE constexpr
    T* begin () requires (is_Unique) { return impl.data; }
    T* mut_begin () requires (supports_owned) {
        make_unique();
        return impl.data;
    }

    ALWAYS_INLINE constexpr
    const T* end () const { return impl.data + size(); }
    ALWAYS_INLINE constexpr
    const T* cend () const { return impl.data + size(); }
    ALWAYS_INLINE constexpr
    T* end () requires (is_Unique) { return impl.data + size(); }
    T* mut_end () requires (supports_owned) {
        make_unique();
        return impl.data + size();
    }

    ALWAYS_INLINE constexpr
    std::reverse_iterator<const T*> rbegin () const {
        return std::make_reverse_iterator(impl.data + size());
    }
    ALWAYS_INLINE constexpr
    std::reverse_iterator<const T*> crbegin () const {
        return std::make_reverse_iterator(impl.data + size());
    }
    ALWAYS_INLINE constexpr
    std::reverse_iterator<T*> rbegin () requires (is_Unique) {
        return std::make_reverse_iterator(impl.data + size());
    }
    constexpr
    std::reverse_iterator<T*> mut_rbegin () requires (supports_owned) {
        make_unique();
        return std::make_reverse_iterator(impl.data + size());
    }

    ALWAYS_INLINE constexpr
    std::reverse_iterator<const T*> rend () const {
        return std::make_reverse_iterator(impl.data);
    }
    ALWAYS_INLINE constexpr
    std::reverse_iterator<const T*> crend () const {
        return std::make_reverse_iterator(impl.data);
    }
    ALWAYS_INLINE constexpr
    std::reverse_iterator<T*> rend () requires (is_Unique) {
        return std::make_reverse_iterator(impl.data);
    }
    constexpr
    std::reverse_iterator<T*> mut_rend () requires (supports_owned) {
        make_unique();
        return std::make_reverse_iterator(impl.data);
    }

    ///// MUTATORS

    constexpr
    void clear () { remove_ref(); impl = {}; }

     // Make sure the array is both unique and has at least enough room for the
     // given number of elements.  The final capacity might be slightly higher
     // than the requested capacity.  Never reduces capacity (use shrink_to_fit
     // to do that).  Calling with a capacity of 1 has the effect of requesting
     // the minimum owned capacity.
    constexpr
    void reserve (usize cap) requires (supports_owned) {
        expect(cap <= max_size_);
        if (!unique() || cap > capacity()) {
            set_as_unique(reallocate(impl, cap), size());
        }
    }

     // Like reserve(), but if reallocation is needed, it doubles the capacity.
    constexpr
    void reserve_plenty (usize cap) requires (supports_owned) {
        expect(cap <= max_size_);
        if (!unique() || cap > capacity())
            [[unlikely]]
        {
            set_as_unique(reallocate(impl, cap, capacity() * 2), size());
        }
    }

     // Make this array unique and if it has more capacity than necessary,
     // reallocate so that capacity is equal to length (rounded up to allocation
     // granularity).
    constexpr
    void shrink_to_fit () requires (supports_owned) {
        if (!unique() || capacity_for_size(size()) < capacity()) {
            if constexpr (std::is_copy_constructible_v<T>) {
                set_as_unique(reallocate(impl, size()), size());
            }
            else never();
        }
    }

     // If this array is not uniquely owned, copy its buffer so it is uniquely
     // owned.  This is equivalent to casting to a UniqueArray/UniqueString and
     // assigning it back to self.
    void make_unique () requires (supports_owned) {
        if (!unique()) {
            set_as_unique(reallocate(impl, size()), size());
        }
    }

     // Change the size of the array.  When growing, default-constructs new
     // elements, and will reallocate if the new size is larger than the current
     // capacity.  When shrinking, destroys elements past the new size, and
     // never reallocates.  If the current array is not unique, makes it unique
     // UNLESS it's shared and the elements are trivially destructible (which
     // means that arrays can share the same buffer despite having different
     // lengths).
    ALWAYS_INLINE
    void resize (usize new_size) requires (
        supports_owned && std::is_default_constructible_v<T>
    ) {
        usize old_size = size();
        if (new_size < old_size) shrink(new_size);
        else if (new_size > old_size) grow(new_size);
    }
    void grow (usize new_size) requires (
        supports_owned && std::is_default_constructible_v<T>
    ) {
        usize old_size = size();
        if (new_size <= old_size) [[unlikely]] return;
        reserve(new_size);
        usize i = old_size;
        try {
            for (; i < new_size; ++i) {
                new ((void*)&impl.data[i]) T();
            }
        }
        catch (...) {
            while (i > old_size) {
                impl.data[i].~T();
            }
            throw;
        }
        set_size(new_size);
    }
     // Not all array classes can grow, but they can all shrink.
    constexpr
    void shrink (usize new_size) requires (supports_owned) {
        if (new_size >= size()) [[unlikely]] return;
        if ((!is_String && std::is_trivially_destructible_v<T>) || !owned()) {
             // Decrease length directly without reallocating.  This can be done
             // even on shared arrays!  But we won't do it for shared strings.
             // See c_str() for why.
            set_size(new_size);
        }
        else if (unique()) {
            for (usize i = size(); i > new_size;) {
                impl.data[--i].~T();
            }
            set_size(new_size);
        }
        else if constexpr (std::is_copy_constructible_v<T>) {
            UniqueArray<T> temp;
            temp.set_as_copy(impl.data, new_size);
            *this = move(temp);
        }
        else never();
    }

     // Construct an element on the end of the array, increasing its size by 1.
    template <class... Args>
    T& emplace_back (Args&&... args) requires (supports_owned) {
        reserve_plenty(size() + 1);
        T& r = *new ((void*)&impl.data[size()]) T(std::forward<Args>(args)...);
        add_size(1);
        return r;
    }

     // emplace_back but skip the capacity and uniqueness check.
    template <class... Args>
    T& emplace_back_expect_capacity (Args&&... args) requires (supports_owned) {
        expect(size() + 1 <= max_size_);
        expect(unique() && capacity() > size());
        T& r = *new ((void*)&impl.data[size()]) T(std::forward<Args>(args)...);
        add_size(1);
        return r;
    }

     // Copy-construct onto the end of the array, increasing its size by 1.
    ALWAYS_INLINE
    T& push_back (const T& v) requires (supports_owned) {
        return emplace_back(v);
    }
     // Move-construct onto the end of the array, increasing its size by 1.
    ALWAYS_INLINE
    T& push_back (T&& v) requires (supports_owned) {
        return emplace_back(move(v));
    }
    ALWAYS_INLINE
    T& push_back_expect_capacity (const T& v) requires (supports_owned) {
        return emplace_back_expect_capacity(v);
    }
    ALWAYS_INLINE
    T& push_back_expect_capacity (T&& v) requires (supports_owned) {
        return emplace_back_expect_capacity(move(v));
    }

    ALWAYS_INLINE
    void pop_back () {
        expect(size() > 0);
        shrink(size() - 1);
    }

     // Append multiple elements by copying them.
    template <ArrayIterator Ptr>
    void append (Ptr p, usize s) requires (
        supports_owned && std::is_copy_constructible_v<T>
    ) {
        reserve_plenty(size() + s);
        copy_fill(impl.data + size(), move(p), s);
        add_size(s);
    }
    void append (SelfSlice o) requires (
        supports_owned && std::is_copy_constructible_v<T>
    ) {
        append(o.data(), o.size());
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End>
    void append (Begin b, End e) requires (
        supports_owned && std::is_copy_constructible_v<T>
    ) {
        if constexpr (requires { usize(e - b); }) {
            return append(move(b), usize(e - b));
        }
        else if constexpr (ArrayForwardIterator<Begin>) {
            usize s = 0;
            for (auto p = b; p != e; ++p) ++s;
            return append(move(b), s);
        }
        else {
            for (auto p = move(b); p != e; ++p) {
                push_back(*b);
            }
        }
    }

     // Append but skip the capacity check
    template <ArrayIterator Ptr>
    void append_expect_capacity (Ptr p, usize s) requires (
        supports_owned && std::is_copy_constructible_v<T>
    ) {
        expect(size() + s <= max_size_);
        expect(unique() && capacity() >= size() + s);
        copy_fill(impl.data + size(), move(p), s);
        add_size(s);
    }
    void append_expect_capacity (SelfSlice o) requires (
        supports_owned && std::is_copy_constructible_v<T>
    ) {
        append_expect_capacity(o.data(), o.size());
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End>
    void append_expect_capacity (Begin b, End e) requires (
        supports_owned && std::is_copy_constructible_v<T>
    ) {
        if constexpr (requires { usize(e - b); }) {
            return append_expect_capacity(move(b), usize(e - b));
        }
        else if constexpr (ArrayForwardIterator<Begin>) {
            usize s = 0;
            for (auto p = b; p != e; ++p) ++s;
            return append_expect_capacity(move(b), s);
        }
        else {
            for (auto p = move(b); p != e; ++p) {
                push_back_expect_capacity(*b);
            }
        }
    }

     // Construct an element into a specific place in the array, moving the rest
     // of the array over by 1.  If the selected constructor is noexcept,
     // constructs the element in-place, otherwise constructs it elsewhere then
     // moves it into the slot.
    template <class... Args>
    T& emplace (usize offset, Args&&... args) requires (supports_owned) {
        expect(offset < size());
        if constexpr (noexcept(T(std::forward<Args>(args)...))) {
            T* dat = do_split(impl, offset, 1);
            T* r = new ((void*)&dat[offset]) T(std::forward<Args>(args)...);
            set_as_unique(dat, size() + 1);
            return *r;
        }
        else {
            T v {std::forward<Args>(args)...};
            T* dat = do_split(impl, offset, 1);
            T* r;
            try {
                r = new ((void*)&dat[offset]) T(move(v));
            }
            catch (...) { never(); }
            set_as_unique(dat, size() + 1);
            return *r;
        }
    }
    template <class... Args>
    T& emplace (const T* pos, Args&&... args) requires (supports_owned) {
        return emplace(pos - impl.data, std::forward<Args>(args)...);
    }
     // Single-element insert, equivalent to emplace.
    T& insert (usize offset, const T& v) requires (supports_owned) {
        return emplace(offset, v);
    }
    T& insert (const T* pos, const T& v) requires (supports_owned) {
        return emplace(pos - impl.data, v);
    }
    T& insert (usize offset, T&& v) requires (supports_owned) {
        return emplace(offset, move(v));
    }
    T& insert (const T* pos, T&& v) requires (supports_owned) {
        return emplace(pos - impl.data, move(v));
    }

     // Multiple-element insert.  If any of the iterator operators or the copy
     // constructor throw, the program will crash.  This is the one exception to
     // the mostly-strong exception guarantee.
    template <ArrayIterator Ptr>
    void insert (usize offset, Ptr p, usize s) requires (
        supports_owned && std::is_copy_constructible_v<T>
    ) {
        expect(offset < size());
        if (s == 0) {
            make_unique();
        }
        else {
            T* dat = do_split(impl, offset, s);
            try {
                copy_fill(dat + offset, move(p), s);
            }
            catch (...) { require(false); }
            set_as_unique(dat, size() + s);
        }
    }
    template <ArrayIterator Ptr>
    void insert (const T* pos, Ptr p, usize s) requires (
        supports_owned && std::is_copy_constructible_v<T>
    ) {
        insert(pos - impl.data, move(p), s);
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End>
    void insert (usize offset, Begin b, End e) requires (
        supports_owned && std::is_copy_constructible_v<T>
    ) {
        usize s;
        if constexpr (requires { usize(e - b); }) {
            s = usize(e - b);
        }
        else {
            static_assert(ArrayForwardIterator<Begin>,
                "Cannot call insert with non-sizeable single-use iterator"
            );
            s = 0; for (auto p = b; p != e; ++p) ++s;
        }
        insert(offset, move(b), s);
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End>
    void insert (const T* pos, Begin b, End e) requires (
        supports_owned && std::is_copy_constructible_v<T>
    ) {
        insert(pos - impl.data, move(b), move(e));
    }

     // Removes element(s) from the array.  If there are elements after the
     // removed ones, they will be move-assigned onto the erased elements,
     // otherwise the erased elements will be destroyed.
    void erase (usize offset, usize count = 1) requires (supports_owned) {
        if (count == 0) {
            make_unique();
        }
        else {
            set_as_unique(do_erase(impl, offset, count), size() - count);
        }
    }
    void erase (const T* pos, usize count = 1) requires (supports_owned) {
        if (count == 0) {
            make_unique();
        }
        else {
            set_as_unique(do_erase(impl, pos - impl.data, count), size() - count);
        }
    }
    void erase (const T* b, const T* e) requires (supports_owned) {
        expect(e >= b);
        if (e - b == 0) {
            make_unique();
        }
        else {
            set_as_unique(do_erase(impl, b - impl.data, e - b), size() - (e - b));
        }
    }

    ///// INTERNAL STUFF

  private:
    ALWAYS_INLINE
    ArrayOwnedHeader& header () const {
        expect(supports_owned);
        return *ArrayOwnedHeader::get(impl.data);
    }

    ALWAYS_INLINE constexpr
    void set_as_owned (T* d, usize s) {
        static_assert(supports_owned);
        expect(s <= max_size_);
        if constexpr (is_Any) {
             // If data is null, clear the owned bit so we can get away with
             // only doing one branch in owned() instead of two.
            impl.sizex2_with_owned = s << 1 | !!d;
        }
        else impl.size = s;
        impl.data = d;
    }
    ALWAYS_INLINE constexpr
    void set_as_unique (T* d, usize s) {
        set_as_owned(d, s);
        expect(unique());
    }
    ALWAYS_INLINE constexpr
    void set_as_unowned (const T* d, usize s) {
        static_assert(supports_static || is_Slice);
        if constexpr (is_Any) {
            impl.sizex2_with_owned = s << 1;
            impl.data = const_cast<T*>(d);
        }
        else {
            impl.size = s;
            impl.data = d;
        }
    }
    template <ArrayIterator Ptr>
    void set_as_copy (Ptr ptr, usize s) requires (std::is_copy_constructible_v<T>) {
        if (s == 0) {
            impl = {}; return;
        }
        T* dat;
        if constexpr (ArrayContiguousIteratorFor<Ptr, T>) {
            dat = allocate_copy(std::to_address(move(ptr)), s);
        }
        else {
             // don't noinline if we can't depolymorph ptr
            dat = allocate_owned(s);
            try {
                copy_fill(dat, move(ptr), s);
            }
            catch (...) {
                deallocate_owned(dat);
                throw;
            }
        }
        set_as_unique(dat, s);
        expect(!header().ref_count);
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End>
    void set_as_copy (Begin b, End e) requires (std::is_copy_constructible_v<T>) {
        if constexpr (requires { usize(e - b); }) {
            set_as_copy(move(b), usize(e - b));
        }
        else if constexpr (requires { b = b; }) {
             // If the iterator is copy-assignable that means it probably allows
             // determining its length in a separate pass.
            usize s = 0;
            for (auto p = b; p != e; ++p) ++s;
            set_as_copy(move(b), s);
        }
        else {
             // You gave us an iterator pair that can't be subtracted and can't
             // be copied.  Guess we'll have to keep reallocating the buffer
             // until it's big enough.
            impl = {};
            try {
                for (auto p = move(b); p != e; ++p) {
                    emplace_back(*p);
                }
            }
            catch (...) {
                this->~Self();
                impl = {};
                throw;
            }
        }
    }
    ALWAYS_INLINE constexpr
    void set_size (usize s) {
        if constexpr (is_Any) {
            impl.sizex2_with_owned = s << 1 | (impl.sizex2_with_owned & 1);
        }
        else impl.size = s;
    }
     // Just an optimization for AnyArray
    ALWAYS_INLINE constexpr
    void add_size (usize change) {
        if constexpr (is_Any) {
            impl.sizex2_with_owned += change << 1;
        }
        else impl.size += change;
    }

    constexpr
    void add_ref () {
        if constexpr (supports_share) {
            if (owned()) {
                ++header().ref_count;
            }
        }
    }

    constexpr
    void remove_ref () {
        if (owned()) {
            if constexpr (is_Unique) {
                expect(header().ref_count == 0);
            }
            else if constexpr (supports_owned) {
                if (header().ref_count) {
                    --header().ref_count;
                    return;
                }
            }
             // Noinline this for shared arrays with nontrivial element
             // destructors.  We don't want to inline an entire loop of
             // destructor calls in every place we remove a reference.
            if constexpr (
                supports_share && !std::is_trivially_destructible_v<T>
            ) noinline_destroy(impl);
            else destroy(impl);
        }
    }
    static
    void destroy (ArrayImplementation<ac, T> impl) {
        Self& self = reinterpret_cast<Self&>(impl);
        for (usize i = self.size(); i > 0;) {
            impl.data[--i].~T();
        }
        deallocate_owned(impl.data);
    }
    NOINLINE static
    void noinline_destroy (ArrayImplementation<ac, T> impl) noexcept {
        destroy(impl);
    }

    static constexpr
    usize capacity_for_size (usize s) {
        usize min_bytes = sizeof(usize) == 8 ? 24 : 16;
        usize min_cap = min_bytes / sizeof(T);
        if (!min_cap) min_cap = 1;
         // Give up on rounding up non-power-of-two sizes, it's not worth it.
        usize mask = sizeof(T) == 1 ? 7
                   : sizeof(T) == 2 ? 3
                   : sizeof(T) == 4 ? 1
                   : 0;
        return s <= min_cap ? min_cap : ((s + mask) & ~mask);
    }

    [[gnu::malloc, gnu::returns_nonnull]] static
    T* allocate_owned (usize s) {
        require(s <= max_size_);
        usize cap = capacity_for_size(s);
         // On 32-bit platforms we need to make sure we don't overflow usize
        uint64 bytes = sizeof(ArrayOwnedHeader) + (uint64)cap * sizeof(T);
        require(bytes <= usize(-1));
        auto header = (ArrayOwnedHeader*)std::malloc(bytes);
        const_cast<uint32&>(header->capacity) = cap;
        header->ref_count = 0;
        return (T*)(header + 1);
    }
    static
    void deallocate_owned (T* d) {
        std::free(ArrayOwnedHeader::get(d));
    }

    template <ArrayIterator Ptr> static
    T* copy_fill (T* dat, Ptr ptr, usize s) requires (std::is_copy_constructible_v<T>) {
        usize i = 0;
        try {
            for (auto p = move(ptr); i < s; ++i, ++p) {
                new ((void*)&dat[i]) T(*p);
            }
        }
        catch (...) {
             // You threw from the copy constructor!  Now we have to clean up
             // the mess.
            while (i > 0) {
                dat[--i].~T();
            }
            throw;
        }
        return dat;
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End> static
    T* copy_fill (T* dat, Begin b, End e) requires (std::is_copy_constructible_v<T>) {
        static_assert(ArrayForwardIterator<Begin>);
        if constexpr (requires { usize(e - b); }) {
            return copy_fill(dat, move(b), usize(e - b));
        }
        else {
            usize s = 0;
            for (auto p = b; p != e; ++p) ++s;
            return copy_fill(dat, move(b), s);
        }
    }

     // This is frequently referenced on non-fast-paths, so it's worth
     // noinlining it.
    [[gnu::malloc, gnu::returns_nonnull]] NOINLINE static
    T* allocate_copy (const T* d, usize s)
        noexcept(std::is_nothrow_copy_constructible_v<T>) requires (std::is_copy_constructible_v<T>)
    {
        expect(s > 0);
        T* dat = allocate_owned(s);
        try {
            return copy_fill(dat, d, s);
        }
        catch (...) {
            deallocate_owned(dat);
            throw;
        }
    }

     // Used by reserve and related functions
    [[gnu::malloc, gnu::returns_nonnull]] NOINLINE static
    T* reallocate (ArrayImplementation<ac, T> impl, usize cap, usize cap2 = 0)
        noexcept(is_Unique || std::is_nothrow_copy_constructible_v<T>)
    {
         // Stuff non-fast-path reserve_plenty logic in here
        if (cap2 > cap) {
            cap = cap2;
            if (cap > max_size_) [[unlikely]] cap = max_size_;
        }
        Self& self = reinterpret_cast<Self&>(impl);
        T* dat = allocate_owned(cap);
         // Can't call deallocate_owned on nullptr.
        if (!self.impl.data) return dat;
        if (self.unique()) {
             // Assume that the move constructor and destructor never throw
             // even if they aren't marked noexcept.
            try {
                for (usize i = 0; i < self.size(); ++i) {
                    new ((void*)&dat[i]) T(move(self.impl.data[i]));
                    self.impl.data[i].~T();
                }
            }
            catch (...) { never(); }
             // DON'T call remove_ref here because it'll double-destroy
             // self.impl.data[*]
            deallocate_owned(self.impl.data);
        }
        else if constexpr (std::is_copy_constructible_v<T>) {
            try {
                copy_fill(dat, self.impl.data, self.size());
            }
            catch (...) {
                deallocate_owned(dat);
                throw;  // -Wno-terminate to suppress warning
            }
            --self.header().ref_count;
        }
        else never();
        return dat;
    }

     // Used by emplace and insert.  Opens a gap but doesn't put anything in it.
     // The caller must placement-new elements in the gap.
    [[gnu::returns_nonnull]] NOINLINE static
    T* do_split (ArrayImplementation<ac, T> impl, usize split, usize shift)
        noexcept(is_Unique || std::is_nothrow_copy_constructible_v<T>)
    {
        Self& self = reinterpret_cast<Self&>(impl);
        expect(split <= self.size());
        expect(shift != 0);
        usize cap = self.capacity();
        if (self.unique() && cap >= self.size() + shift) {
             // We have enough capacity so all we need to do is move the tail.
             // Assume that the move constructor and destructor never throw even
             // if they aren't marked noexcept.
            try {
                 // Move elements forward, starting at the back
                for (usize i = self.size(); i > split; --i) {
                    new ((void*)&self.impl.data[i-1 + shift]) T(
                        move(self.impl.data[i-1])
                    );
                    self.impl.data[i-1].~T();
                }
            }
            catch (...) { never(); }
            return self.impl.data;
        }
         // Not enough capacity!  We have to reallocate, and while we're at it,
         // let's do the copy/move too.
        T* dat = allocate_owned(
            cap * 2 > self.size() + shift ?
            cap * 2 : self.size() + shift
        );
        if (cap > max_size_) [[unlikely]] cap = max_size_;
        if (self.unique()) {
             // Assume that the move constructor and destructor never throw even
             // if they aren't marked noexcept.
            try {
                for (usize i = 0; i < split; ++i) {
                    new ((void*)&dat[i]) T(move(self.impl.data[i]));
                    self.impl.data[i].~T();
                }
                for (usize i = 0; i < self.size() - split; ++i) {
                    new ((void*)&dat[split + shift + i]) T(
                        move(self.impl.data[split + i])
                    );
                    self.impl.data[split + i].~T();
                }
            }
            catch (...) { never(); }
             // Don't use remove_ref, it'll call the destructors again
            deallocate_owned(self.impl.data);
        }
        else if constexpr (std::is_copy_constructible_v<T>) { // Not unique
            usize head_i = 0;
            usize tail_i = split;
            try {
                for (; head_i < split; ++head_i) {
                    new ((void*)&dat[head_i]) T(self.impl.data[head_i]);
                }
                for (; tail_i < self.size(); ++tail_i) {
                    new ((void*)&dat[shift + tail_i]) T(
                        self.impl.data[tail_i]
                    );
                }
            }
            catch (...) {
                 // Yuck, someone threw an exception in a copy constructor!
                while (tail_i > split) {
                    dat[shift + --tail_i].~T();
                }
                while (head_i > 0) {
                    dat[--head_i].~T();
                }
                throw;
            }
            --self.header().ref_count;
        }
        else never();
        return dat;
    }

    [[gnu::returns_nonnull]] NOINLINE static
    T* do_erase (ArrayImplementation<ac, T> impl, usize offset, usize count)
        noexcept(is_Unique || std::is_nothrow_copy_constructible_v<T>)
    {
        Self& self = reinterpret_cast<Self&>(impl);
        usize old_size = self.size();
        expect(count != 0);
        expect(offset <= old_size && offset + count <= old_size);
        if (self.unique()) {
            try {
                 // Move some elements over.  The destination will always
                 // still exist so use operator=
                for (usize i = offset; count + i < old_size; ++i) {
                    self.impl.data[i] = move(
                        self.impl.data[count + i]
                    );
                }
                 // Then delete the rest
                for (usize i = old_size - count; i < old_size; ++i) {
                    self.impl.data[i].~T();
                }
            }
            catch (...) { never(); }
            return self.impl.data;
        }
        else if constexpr (std::is_copy_constructible_v<T>) {
             // Not unique, so copy instead of moving
            T* dat = allocate_owned(old_size - count);
            usize i = 0;
            try {
                for (; i < offset; i++) {
                    new ((void*)&dat[i]) T(self.impl.data[i]);
                }
                for (; i < old_size - count; i++) {
                    new ((void*)&dat[i]) T(self.impl.data[count + i]);
                }
            }
            catch (...) {
                 // If an exception happens we have to destroy the
                 // incompletely constructed target array.  Fortunately,
                 // unlike in do_split, the target array is completely
                 // contiguous.
                while (i > 0) {
                    dat[--i].~T();
                }
                throw;
            }
            --self.header().ref_count;
            return dat;
        }
        else never();
    }
};

///// OPERATORS

 // Make this template as generic as possible but nail one side down to
 // ArrayInterface.
template <ArrayClass ac, class T, class B>
bool operator== (
    const ArrayInterface<ac, T>& a, const B& b
) requires (
    requires { usize(b.size()); a.data()[0] == b.data()[0]; }
) {
    usize as = a.size();
    usize bs = b.size();
    const T* ad = a.data();
    const auto& bd = b.data();
    if (as != bs) return false;
     // Unlike most STL containers, this WILL short-circuit if the arrays have
     // the same data pointer and size.
    if constexpr (requires { ad == bd; }) {
        if (ad == bd) return true;
    }
    for (usize i = 0; i < as; ++i) {
        if (!(ad[i] == bd[i])) {
            return false;
        }
    }
    return true;
}
 // Allow comparing to NUL-terminated pointer for string types only.
template <ArrayClass ac, class T>
bool operator== (
    const ArrayInterface<ac, T>& a, const T* b
) requires (ArrayInterface<ac, T>::is_String) {
    usize as = a.size();
    const T* ad = a.data();
     // Can't short-circuit if ad == b because b might be NUL-terminated early.
    usize i;
    for (i = 0; i < as; ++i) {
        if (!b[i] || !(ad[i] == b[i])) {
            return false;
        }
    }
    return !b[i];
}

 // I can't be bothered to learn what <=> is supposed to return.  They should
 // have just made it int.
template <ArrayClass ac, class T, class B>
auto operator<=> (
    const ArrayInterface<ac, T>& a, const B& b
) requires (
    requires { usize(b.size()); a.data()[0] <=> b.data()[0]; }
) {
    usize as = a.size();
    usize bs = b.size();
    const T* ad = a.data();
    const auto& bd = b.data();
    if constexpr (requires { ad == bd; }) {
        if (as == bs && ad == bd) return 0 <=> 0;
    }
    for (usize i = 0; i < as && i < bs; ++i) {
        auto res = ad[i] <=> bd[i];
        if (res != (0 <=> 0)) return res;
    }
    return as <=> bs;
}
template <ArrayClass ac, class T>
auto operator<=> (
    const ArrayInterface<ac, T>& a, const T* b
) requires (ArrayInterface<ac, T>::is_String) {
    const T* ad = a.data();
    if (ad == b) return 0 <=> 0;
    usize as = a.size();
    for (usize i = 0; i < as && b[i]; ++i) {
        auto res = ad[i] <=> b[i];
        if (res != (0 <=> 0)) return res;
    }
    return 0 <=> 0;
}

} // arrays
} // uni

namespace std {
template <uni::ArrayClass ac, class T>
struct hash<uni::ArrayInterface<ac, T>> {
    uni::usize operator() (const uni::ArrayInterface<ac, T>& a) const {
         // Just do an x33 hash (djb2) on whatever std::hash returns for the
         // contents.  At least for libstdc++, hash is a no-op on basic integer
         // types, so for char strings this just does an x33 hash on the string.
         //
         // This is fast but vulnerable to hash denial attacks.
        uni::usize r = 5381;
        for (auto& e : a) {
            r = (r << 5) + r + std::hash<T>{}(e);
        }
        return r;
    }
};
}
