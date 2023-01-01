 // This is here if you want to access the underlying implementation of the
 // array classes.  These structs have no behavior associated with them, so if
 // you want to deal with them, you have to manage refcounts and such yourself.
#pragma once
#include <iterator>
#include "common.h"

namespace uni {
inline namespace arrays {

///// ARRAY TYPES 
enum class ArrayClass {
    AnyA,
    AnyS,
    StaticA,
    StaticS,
    SharedA,
    SharedS,
    UniqueA,
    UniqueS,
    SliceA,
    SliceS
};

///// SHARED HEADER
 // If an array is shared or unique (owned() == true), then this struct is right
 // behind the buffer in memory.  The total allocated size is
 // sizeof(ArrayOwnedHeader) + capacity.

 // Note: Objects with align > 8 are not currently supported.
struct alignas(8) ArrayOwnedHeader {
    const uint32 capacity; // Number of elements this buffer can hold.
    mutable uint32 ref_count; // zero-based (= 0 for unique arrays)
    static ArrayOwnedHeader* get (const void* data) {
        return (ArrayOwnedHeader*)data - 1;
    }
};

///// ARRAY IMPLEMENTATIONS

template <ArrayClass, class>
struct ArrayImplementation;

template <class T>
struct ArrayImplementation<ArrayClass::AnyA, T> {
     // The first bit is owned, the rest is size (shifted left by 1).
     // We are not using bitfields because they are not optimized very well.
    usize sizex2_with_owned;
    T* data;
};

template <class T>
struct ArrayImplementation<ArrayClass::AnyS, T> {
    usize sizex2_with_owned;
    T* data;
};

template <class T>
struct ArrayImplementation<ArrayClass::SharedA, T> {
    usize size;
    T* data;
};
template <class T>
struct ArrayImplementation<ArrayClass::SharedS, T> {
    usize size;
    T* data;
};

template <class T>
struct ArrayImplementation<ArrayClass::UniqueA, T> {
    usize size;
    T* data;
};
template <class T>
struct ArrayImplementation<ArrayClass::UniqueS, T> {
    usize size;
    T* data;
};

template <class T>
struct ArrayImplementation<ArrayClass::StaticA, T> {
    usize size;
    const T* data;
};
template <class T>
struct ArrayImplementation<ArrayClass::StaticS, T> {
    usize size;
    const T* data;
};

template <class T>
struct ArrayImplementation<ArrayClass::SliceA, T> {
    usize size;
    const T* data;
};
template <class T>
struct ArrayImplementation<ArrayClass::SliceS, T> {
    usize size;
    const T* data;
};

} // arrays
} // uni
