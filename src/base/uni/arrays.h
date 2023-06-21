 // Arrays that can be shared (ref-counted) or static
 //
 // This header provides a constellation of array and string classes that share
 // a common interface and differ by ownership model.  They're largely
 // compatible with STL containers, but not quite a drop-in replacement.
 //
 // THREAD-SAFETY
 // SharedArray and AnyArray use reference counting which is not thread-safe.
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
 // copy-on-write behavior.
template <class T>
using AnyArray = ArrayInterface<ArrayClass::AnyA, T>;

 // An array that can only reference shared data.  There isn't much reason to
 // use this instead of AnyArray, but it's here as an intermediate between AnyArray
 // and UniqueArray.
template <class T>
using SharedArray = ArrayInterface<ArrayClass::SharedA, T>;

 // An array that can guarantees unique ownership, allowing mutation without
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
    static constexpr bool supports_copy = supports_share || is_Unique;
    static constexpr bool supports_static = is_Any || is_Static;
    static constexpr bool trivially_copyable = is_Static || is_Slice;

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
    using mut_iterator = std::conditional_t<supports_copy, T*, void>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const T*>;
    using mut_reverse_iterator =
        std::conditional_t<supports_copy, std::reverse_iterator<T*>, void>;

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
     // copies the buffer.  Although move conversion will never fail, it's
     // disabled for StaticArray and Slice, and the copy constructor from
     // AnyArray to StaticArray can fail.
    template <ArrayClass ac2> constexpr
    ArrayInterface (ArrayInterface<ac2, T>&& o) requires (
        !trivially_copyable && !o.trivially_copyable
    ) {
        if (supports_share || o.unique()) {
            set_as_owned(o.impl.data, o.size());
            o.impl = {};
        }
        else {
            set_as_copy(o.impl.data, o.size());
        }
    }
     // Warn about moving from possibly-owned array to Slice, since that's
     // likely a mistake.
    template <ArrayClass ac2> [[deprecated(
        "Warning: Moving from possibly-owned array to Slice could cause the "
        "Slice to reference stale data."
    )]] constexpr
    ArrayInterface (ArrayInterface<ac2, T>&& o) requires (
        is_Slice && !o.trivially_copyable
    ) {
        set_as_unowned(o.data(), o.size());
    }

     // Copy construct.  Always copies the buffer for UniqueArray, never copies
     // the buffer for other array classes.
    constexpr
    ArrayInterface (const ArrayInterface& o) requires (!trivially_copyable) {
        if constexpr (is_Unique) {
            set_as_copy(o.data(), o.size());
        }
        else if constexpr (supports_share) {
            impl = o.impl;
            add_ref();
        }
        else {
            set_as_unowned(o.data(), o.size());
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
        supports_copy || !o.is_Slice
    ) {
        if constexpr (o.is_Slice) {
             // Always copy from Slice because we don't know where it came from
             // or where it's going.
            set_as_copy(o.data(), o.size());
        }
        else if constexpr (is_Slice) {
            set_as_unowned(o.data(), o.size());
        }
        else if constexpr (is_Unique || o.is_Unique) {
            set_as_copy(o.data(), o.size());
        }
        else if (o.owned()) {
            if constexpr (supports_share && o.supports_share) {
                set_as_owned(o.data(), o.size());
                add_ref();
            }
            else {
                 // We're a StaticArray but we've been given non-static data.
                 // There's nothing we can do.
                require(false);
            }
        }
        else if constexpr (supports_static) {
            set_as_unowned(o.data(), o.size());
        }
        else {
            set_as_copy(o.data(), o.size());
        }
    }

     // However, at compile-time you can implicity construct a StaticArray from
     // a Slice, since it's nearly guaranteed to be static data (and if it
     // isn't, you'll get a compile error).
    template <ArrayClass ac2> consteval
    ArrayInterface (const ArrayInterface<ac2, T>& o) requires (
        is_Static && o.is_Slice
    ) {
        set_as_unowned(o.data(), o.size());
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
    template <ArrayIterator Ptr> constexpr
    ArrayInterface (Ptr p, usize s) requires (!is_Static) {
        if constexpr (is_Slice) {
            static_assert(ArrayIteratorFor<Ptr, T>,
                "Cannot borrow Slice from iterator with non-exact element type."
            );
            static_assert(ArrayContiguousIterator<Ptr>,
                "Cannot borrow Slice from non-contiguous iterator."
            );
            set_as_unowned(std::to_address(std::move(p)), s);
        }
        else {
            set_as_copy(std::move(p), s);
        }
    }
    template <ArrayIterator Ptr> consteval
    ArrayInterface (Ptr p, usize s) requires (is_Static) {
        static_assert(ArrayIteratorFor<Ptr, T>,
            "Cannot construct static array from iterator with non-exact "
            "element type."
        );
        static_assert(ArrayContiguousIterator<Ptr>,
            "Cannot construct static array from non-contiguous iterator."
        );
        set_as_unowned(std::to_address(std::move(p)), s);
    }

     // Construct from a pair of iterators.
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End> constexpr
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
            set_as_unowned(std::to_address(std::move(b)), usize(e - b));
        }
        else {
            set_as_copy(std::move(b), std::move(e));
        }
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End> consteval
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
    ArrayInterface (usize s) requires (supports_copy) {
        if (!s) {
            impl = {}; return;
        }
        T* dat = allocate_owned(s);
        usize i = 0;
        try {
            for (; i < s; ++i) {
                new (&dat[i]) T();
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
     // only available for owned array classes.
    ArrayInterface (usize s, const T& v) requires (supports_copy) {
        if (!s) {
            impl = {}; return;
        }
        T* dat = allocate_owned(s);
        usize i = 0;
        try {
            for (; i < s; ++i) {
                new (&dat[i]) T(v);
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

     // Finally, constructing from a std::intiializer_list always copies because
     // that's the only thing you can do with an initializer list.
    ArrayInterface (std::initializer_list<T> l) {
        static_assert(supports_copy,
            "Can't construct a non-owning array from a std::initializer_list."
        );
        set_as_copy(std::data(l), std::size(l));
    }

    ///// NAMED CONSTRUCTORS
    // These allow you to explicitly specify a construction method.

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
    Self Copy (SelfSlice o) requires (supports_copy) {
        return UniqueArray<T>(o);
    }
    template <ArrayIterator Ptr> static
    Self Copy (Ptr p, usize s) requires (supports_copy) {
        return UniqueArray<T>(std::move(p), s);
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End> static
    Self Copy (Begin b, End e) requires (supports_copy) {
        return UniqueArray<T>(std::move(b), std::move(e));
    }

     // Explicitly reference static data.  Unlike the consteval constructors,
     // these are allowed at run time.  It's the caller's responsibility to make
     // sure that the referenced data outlives both the returned StaticArray AND
     // all AnyArrays and StaticArrays that may be constructed from it.
    static constexpr
    Self Static (SelfSlice o) requires (supports_static) {
        static_assert(ArrayIteratorFor<decltype(ArrayLike_data(o)), T>,
            "Cannot construct static array from array-like type with non-exact "
            "element type."
        );
        static_assert(ArrayContiguousIterator<decltype(ArrayLike_data(o))>,
            "Cannot construct static array from array-like type if its data() "
            "returns a non-contiguous iterator."
        );
        StaticArray<T> r;
        r.set_as_unowned(o.data(), o.size());
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
        r.set_as_unowned(std::to_address(std::move(p)), s);
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

    constexpr
    ArrayInterface& operator= (ArrayInterface&& o) requires (
        !trivially_copyable
    ) {
        if (&o == this) [[unlikely]] return *this;
        remove_ref();
        impl = o.impl;
        o.impl = {};
        return *this;
    }
    constexpr
    ArrayInterface& operator= (const ArrayInterface& o) requires (
        !trivially_copyable
    ) {
        if (&o == this) [[unlikely]] return *this;
        remove_ref();
        impl = o.impl;
        add_ref();
        return *this;
    }
    constexpr
    ArrayInterface& operator= (const ArrayInterface&) requires (
        trivially_copyable
    ) = default;

    ///// COERCION

     // Okay okay
    ALWAYS_INLINE constexpr
    operator std::string_view () const requires (is_String) {
        return std::string_view(data(), size());
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
        supports_copy ? 0x7fff'fff0 : usize(-1) >> 1;
    ALWAYS_INLINE constexpr
    usize max_size () const { return max_size_; }

     // Get data pointer.  Always const by default except for UniqueArray, to
     // avoid accidental copy-on-write.
    ALWAYS_INLINE constexpr
    const T* data () const { return impl.data; }
    ALWAYS_INLINE
    T* data () requires (is_Unique) { return impl.data; }
     // Get mutable data pointer, triggering copy-on-write if needed.
    T* mut_data () requires (supports_copy) {
        make_unique();
        return impl.data;
    }

     // Guarantees the return of a NUL-terminated buffer, possibly by attaching
     // a NUL element after the end.  Does not change the size of the array, but
     // may change its capacity.  For StaticArray and Slice, since they can't be
     // mutated, this require()s that the array is explicitly NUL-terminated.
    constexpr
    const T* c_str () requires (requires { T(0) == 0; }) {
        if (size() > 0 && impl.data[size()-1] == 0) {
            return impl.data;
        }
        else if constexpr (supports_copy) {
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
            impl.data[size()] = 0;
            return impl.data;
        }
        else require(false);
    }

     // Access individual elements.  at() and mut_at() will terminate if the
     // passed index is out of bounds.  operator[], get(), and mut_get() do not
     // do bounds checks (except on debug builds).  Only the mut_* versions can
     // trigger copy-on-write; the non-mut_* version are always const except for
     // UniqueArray.
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
    T& mut_at (usize i) requires (supports_copy) {
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
    T& mut_get (usize i) requires (supports_copy) {
        make_unique();
        return const_cast<T&>((*this)[i]);
    }

    ALWAYS_INLINE constexpr
    const T& front () const { return (*this)[0]; }
    ALWAYS_INLINE constexpr
    T& front () const requires (is_Unique) { return (*this)[0]; }
    T& mut_front () const requires (supports_copy) {
        make_unique();
        return (*this)[0];
    }

    ALWAYS_INLINE constexpr
    const T& back () const { return (*this)[size() - 1]; }
    ALWAYS_INLINE constexpr
    T& back () const requires (is_Unique) { return (*this)[size() - 1]; }
    T& mut_back () const requires (supports_copy) {
        make_unique();
        return (*this)[size() - 1];
    }

     // Like substr. but doesn't do bounds checking.
    constexpr
    SelfSlice slice (usize offset, usize length) const {
        expect(offset < size() && offset + length < size());
        return SelfSlice(data() + offset, length);
    }
     // Like slice, but capped at the end of the contents.
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
        else if constexpr (supports_copy) {
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
        else if constexpr (supports_copy) {
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
    T* mut_begin () requires (supports_copy) {
        make_unique();
        return impl.data;
    }

    ALWAYS_INLINE constexpr
    const T* end () const { return impl.data + size(); }
    ALWAYS_INLINE constexpr
    const T* cend () const { return impl.data + size(); }
    ALWAYS_INLINE constexpr
    T* end () requires (is_Unique) { return impl.data + size(); }
    T* mut_end () requires (supports_copy) {
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
    std::reverse_iterator<T*> mut_rbegin () requires (supports_copy) {
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
    std::reverse_iterator<T*> mut_rend () requires (supports_copy) {
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
    void reserve (usize cap) requires (supports_copy) {
        expect(cap <= max_size_);
        if (!unique() || cap > capacity()) {
            set_as_unique(reallocate(impl, cap), size());
        }
    }

     // Like reserve(), but if reallocation is needed, it doubles the capacity.
    constexpr
    void reserve_plenty (usize cap) requires (supports_copy) {
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
    void shrink_to_fit () requires (supports_copy) {
        if (!unique() || capacity_for_size(size()) < capacity()) {
            set_as_unique(reallocate(impl, size()), size());
        }
    }

     // If this array is not uniquely owned, copy its buffer so it is uniquely
     // owned.  This is equivalent to casting to a UniqueArray/UniqueString and
     // assigning it back to self.
    void make_unique () requires (supports_copy) {
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
    void resize (usize new_size) requires (supports_copy) {
        usize old_size = size();
        if (new_size < old_size) shrink(new_size);
        else if (new_size > old_size) grow(new_size);
    }
    void grow (usize new_size) requires (supports_copy) {
        usize old_size = size();
        if (new_size <= old_size) [[unlikely]] return;
        reserve(new_size);
        usize i = old_size;
        try {
            for (; i < new_size; ++i) {
                new (&impl.data[i]) T();
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
    void shrink (usize new_size) {
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
        else {
            UniqueArray<T> temp;
            temp.set_as_copy(impl.data, new_size);
            *this = std::move(temp);
        }
    }

     // Construct an element on the end of the array, increasing its size by 1.
    template <class... Args>
    void emplace_back (Args&&... args) requires (supports_copy) {
        reserve_plenty(size() + 1);
        new (&impl.data[size()]) T(std::forward<Args>(args)...);
        add_size(1);
    }

     // emplace_back but skip the capacity and uniqueness check.
    template <class... Args>
    void emplace_back_expect_capacity (Args&&... args) requires (supports_copy) {
        expect(size() + 1 <= max_size_);
        expect(unique() && capacity() > size());
        new (&impl.data[size()]) T(std::forward<Args>(args)...);
        add_size(1);
    }

     // Copy-construct onto the end of the array, increasing its size by 1.
    ALWAYS_INLINE
    void push_back (const T& v) requires (supports_copy) {
        emplace_back(v);
    }
     // Move-construct onto the end of the array, increasing its size by 1.
    ALWAYS_INLINE
    void push_back (T&& v) requires (supports_copy) {
        emplace_back(std::move(v));
    }
    ALWAYS_INLINE
    void push_back_expect_capacity (const T& v) requires (supports_copy) {
        emplace_back_expect_capacity(v);
    }
    ALWAYS_INLINE
    void push_back_expect_capacity (T&& v) requires (supports_copy) {
        emplace_back_expect_capacity(std::move(v));
    }

    ALWAYS_INLINE
    void pop_back () {
        expect(size() > 0);
        shrink(size() - 1);
    }

     // Append multiple elements.
    template <ArrayIterator Ptr>
    void append (Ptr p, usize s) requires (supports_copy) {
        reserve_plenty(size() + s);
        copy_fill(impl.data + size(), std::move(p), s);
        add_size(s);
    }
    void append (SelfSlice o) requires (supports_copy) {
        append(o.data(), o.size());
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End>
    void append (Begin b, End e) requires (supports_copy) {
        if constexpr (requires { usize(e - b); }) {
            return append(std::move(b), usize(e - b));
        }
        else if constexpr (ArrayForwardIterator<Begin>) {
            usize s = 0;
            for (auto p = b; p != e; ++p) ++s;
            return append(std::move(b), s);
        }
        else {
            for (auto p = std::move(b); p != e; ++p) {
                push_back(*b);
            }
        }
    }

     // Append but skip the capacity check
    template <ArrayIterator Ptr>
    void append_expect_capacity (Ptr p, usize s) requires (supports_copy) {
        expect(size() + s <= max_size_);
        expect(unique() && capacity() >= size() + s);
        copy_fill(impl.data + size(), std::move(p), s);
        add_size(s);
    }
    void append_expect_capacity (SelfSlice o) requires (supports_copy) {
        append_expect_capacity(o.data(), o.size());
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End>
    void append_expect_capacity (Begin b, End e) requires (supports_copy) {
        if constexpr (requires { usize(e - b); }) {
            return append_expect_capacity(std::move(b), usize(e - b));
        }
        else if constexpr (ArrayForwardIterator<Begin>) {
            usize s = 0;
            for (auto p = b; p != e; ++p) ++s;
            return append_expect_capacity(std::move(b), s);
        }
        else {
            for (auto p = std::move(b); p != e; ++p) {
                push_back_expect_capacity(*b);
            }
        }
    }

     // Construct an element into a specific place in the array, moving the rest
     // of the array over by 1.  If the selected constructor is noexcept,
     // constructs the element in-place, otherwise constructs it elsewhere then
     // moves it into the slot.
    template <class... Args>
    void emplace (usize offset, Args&&... args) requires (supports_copy) {
        expect(offset < size());
        if constexpr (noexcept(T(std::forward<Args>(args)...))) {
            T* dat = do_split(impl, offset, 1);
            new (&dat[offset]) T(std::forward<Args>(args)...);
            set_as_unique(dat, size() + 1);
        }
        else {
            T v {std::forward<Args>(args)...};
            T* dat = do_split(impl, offset, 1);
            try {
                new (&dat[offset]) T(std::move(v));
            }
            catch (...) { never(); }
            set_as_unique(dat, size() + 1);
        }
    }
    template <class... Args>
    void emplace (const T* pos, Args&&... args) requires (supports_copy) {
        emplace(pos - impl.data, std::forward<Args>(args)...);
    }
     // Single-element insert, equivalent to emplace.
    void insert (usize offset, const T& v) requires (supports_copy) {
        emplace(offset, v);
    }
    void insert (const T* pos, const T& v) requires (supports_copy) {
        emplace(pos - impl.data, v);
    }
    void insert (usize offset, T&& v) requires (supports_copy) {
        emplace(offset, std::move(v));
    }
    void insert (const T* pos, T&& v) requires (supports_copy) {
        emplace(pos - impl.data, std::move(v));
    }

     // Multiple-element insert.  If any of the iterator operators or the copy
     // constructor throw, the program will crash.  This is the one exception to
     // the mostly-strong exception guarantee.
    template <ArrayIterator Ptr>
    void insert (usize offset, Ptr p, usize s) requires (supports_copy) {
        expect(offset < size());
        if (s == 0) {
            make_unique();
        }
        else {
            T* dat = do_split(impl, offset, s);
            try {
                copy_fill(dat + offset, std::move(p), s);
            }
            catch (...) { require(false); }
            set_as_unique(dat, size() + s);
        }
    }
    template <ArrayIterator Ptr>
    void insert (const T* pos, Ptr p, usize s) requires (supports_copy) {
        insert(pos - impl.data, std::move(p), s);
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End>
    void insert (usize offset, Begin b, End e) requires (supports_copy) {
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
        insert(offset, std::move(b), s);
    }
    template <ArrayIterator Begin, ArraySentinelFor<Begin> End>
    void insert (const T* pos, Begin b, End e) requires (supports_copy) {
        insert(pos - impl.data, std::move(b), std::move(e));
    }

     // Removes element(s) from the array.  If there are elements after the
     // removed ones, they will be move-assigned onto the erased elements,
     // otherwise the erased elements will be destroyed.
    void erase (usize offset, usize count = 1) requires (supports_copy) {
        if (count == 0) {
            make_unique();
        }
        else {
            set_as_unique(do_erase(impl, offset, count), size() - count);
        }
    }
    void erase (const T* pos, usize count = 1) requires (supports_copy) {
        if (count == 0) {
            make_unique();
        }
        else {
            set_as_unique(do_erase(impl, pos - impl.data, count), size() - count);
        }
    }
    void erase (const T* b, const T* e) requires (supports_copy) {
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
        expect(supports_copy);
        return *ArrayOwnedHeader::get(impl.data);
    }

    ALWAYS_INLINE constexpr
    void set_as_owned (T* d, usize s) {
        static_assert(supports_copy);
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
    void set_as_copy (Ptr ptr, usize s) {
        if (s == 0) {
            impl = {}; return;
        }
        T* dat;
        if constexpr (ArrayContiguousIteratorFor<Ptr, T>) {
            dat = allocate_copy(std::to_address(std::move(ptr)), s);
        }
        else {
             // don't noinline if we can't depolymorph ptr
            dat = allocate_owned(s);
            try {
                copy_fill(dat, std::move(ptr), s);
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
    void set_as_copy (Begin b, End e) {
        if constexpr (requires { usize(e - b); }) {
            set_as_copy(std::move(b), usize(e - b));
        }
        else if constexpr (requires { b = b; }) {
             // If the iterator is copy-assignable that means it probably allows
             // determining its length in a separate pass.
            usize s = 0;
            for (auto p = b; p != e; ++p) ++s;
            set_as_copy(std::move(b), s);
        }
        else {
             // You gave us an iterator pair that can't be subtracted and can't
             // be copied.  Guess we'll have to keep reallocating the buffer
             // until it's big enough.
            impl = {};
            try {
                for (auto p = std::move(b); p != e; ++p) {
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
            else if constexpr (supports_copy) {
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
    void noinline_destroy (ArrayImplementation<ac, T> impl) { destroy(impl); }

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
        require(bytes < usize(-1));
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
    T* copy_fill (T* dat, Ptr ptr, usize s) {
        usize i = 0;
        try {
            for (auto p = std::move(ptr); i < s; ++i, ++p) {
                new (&dat[i]) T(*p);
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
    T* copy_fill (T* dat, Begin b, End e) {
        static_assert(ArrayForwardIterator<Begin>);
        if constexpr (requires { usize(e - b); }) {
            return copy_fill(dat, std::move(b), usize(e - b));
        }
        else {
            usize s = 0;
            for (auto p = b; p != e; ++p) ++s;
            return copy_fill(dat, std::move(b), s);
        }
    }

     // This is frequently referenced on non-fast-paths, so it's worth
     // noinlining it.
    [[gnu::malloc, gnu::returns_nonnull]] NOINLINE static
    T* allocate_copy (const T* d, usize s) {
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
                    new (&dat[i]) T(std::move(self.impl.data[i]));
                    self.impl.data[i].~T();
                }
            }
            catch (...) { never(); }
             // DON'T call remove_ref here because it'll double-destroy
             // self.impl.data[*]
            deallocate_owned(self.impl.data);
        }
        else {
            try {
                copy_fill(dat, self.impl.data, self.size());
            }
            catch (...) {
                deallocate_owned(dat);
                throw;  // -Wno-terminate to suppress warning
            }
            --self.header().ref_count;
        }
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
                    new (&self.impl.data[i-1 + shift]) T(
                        std::move(self.impl.data[i-1])
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
                    new (&dat[i]) T(std::move(self.impl.data[i]));
                    self.impl.data[i].~T();
                }
                for (usize i = 0; i < self.size() - split; ++i) {
                    new (&dat[split + shift + i]) T(
                        std::move(self.impl.data[split + i])
                    );
                    self.impl.data[split + i].~T();
                }
            }
            catch (...) { never(); }
             // Don't use remove_ref, it'll call the destructors again
            deallocate_owned(self.impl.data);
        }
        else { // Not unique
            usize head_i = 0;
            usize tail_i = 0;
            try {
                for (; head_i < self.size(); ++head_i) {
                    new (&dat[head_i]) T(self.impl.data[head_i]);
                }
                for (; tail_i < cap - self.size(); ++tail_i) {
                    new (&dat[split + shift + tail_i]) T(
                        self.impl.data[split + tail_i]
                    );
                }
            }
            catch (...) {
                 // Yuck, someone threw an exception in a copy constructor!
                while (tail_i > 0) {
                    dat[split + shift + --tail_i].~T();
                }
                while (head_i > 0) {
                    dat[--head_i].~T();
                }
                throw;
            }
            --self.header().ref_count;
        }
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
                    self.impl.data[i] = std::move(
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
        else {
             // Not unique, so copy instead of moving
            T* dat = allocate_owned(old_size - count);
            usize i = 0;
            try {
                for (; i < offset; i++) {
                    new (&dat[i]) T(self.impl.data[i]);
                }
                for (; i < old_size - count; i++) {
                    new (&dat[i]) T(self.impl.data[count + i]);
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
     // Unlike most STL types, this WILL short-circuit if the arrays have the
     // same data pointer.
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
    for (usize i = 0; i < as && b[i]; ++i) {
        if (!(ad[i] == b[i])) {
            return false;
        }
    }
    return true;
}

 // I can't be bothered to learn what <=> is supposed to return.  They should
 // have just made it int.
template <ArrayClass ac, class T, class B>
auto operator<=> (
    const ArrayInterface<ac, T>& a, const B& b
) requires (
    requires { usize(b.size()); a.data()[0] <=> b.data()[0]; }
) {
    const T* ad = a.data();
    const auto& bd = b.data();
    if constexpr (requires { ad == bd; }) {
        if (ad == bd) return 0 <=> 0;
    }
    usize as = a.size();
    usize bs = b.size();
    for (usize i = 0; i < as && i < bs; ++i) {
        auto res = ad[i] <=> bd[i];
        if (res != (0 <=> 0)) return res;
    }
    return 0 <=> 0;
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
