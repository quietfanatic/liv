#pragma once
#include "../common.h"
#include "../exception.h"
#include "../../uni/utf.h"

namespace ayu::in {

using TreeRep = int8;
enum : TreeRep {
    REP_UNDEFINED = 0,
    REP_NULL = 1,
    REP_BOOL = 2,
    REP_INT64 = 3,
    REP_DOUBLE = 4,
    REP_STATICSTRING = 5,
     // Types requiring reference counting
    REP_SHAREDSTRING = -1,
    REP_ARRAY = -2,
    REP_OBJECT = -3,
    REP_ERROR = -4,
};

NOINLINE
void delete_Tree_data (TreeRef);

[[noreturn]]
void bad_Tree_form (TreeRef, TreeForm);

} // ayu::in
namespace ayu {

constexpr Tree::Tree () :
    form(UNDEFINED), rep(0), flags(0), length(0), data{.as_int64 = 0}
{ }
constexpr Tree::Tree (Null) :
    form(NULLFORM), rep(in::REP_NULL), flags(0), length(0), data{.as_int64 = 0}
{ }
 // Use .as_int64 to write all of data
template <class T> requires (std::is_same_v<T, bool>)
constexpr Tree::Tree (T v) :
    form(BOOL), rep(in::REP_BOOL), flags(0), length(0), data{.as_int64 = v}
{ }
template <class T> requires (
    std::is_integral_v<T> &&
    !std::is_same_v<T, bool> && !std::is_same_v<T, char>
)
constexpr Tree::Tree (T v) :
    form(NUMBER), rep(in::REP_INT64), flags(0), length(0), data{.as_int64 = int64(v)}
{ }
template <class T> requires (std::is_floating_point_v<T>)
constexpr Tree::Tree (T v) :
    form(NUMBER), rep(in::REP_DOUBLE), flags(0), length(0), data{.as_double = v}
{ }
constexpr Tree::Tree (AnyString v) :
    form(STRING), rep(v.owned() ? in::REP_SHAREDSTRING : in::REP_STATICSTRING),
    flags(0), length(v.size()), data{.as_char_ptr = v.data()}
{
    require(v.size() <= uint32(-1));
    v.dematerialize();
}
inline Tree::Tree (Str16 v) : Tree(from_utf16(v)) { }
constexpr Tree::Tree (TreeArray v) :
    form(ARRAY), rep(in::REP_ARRAY), flags(0),
    length(v.size()), data{.as_array_ptr = v.data()}
{
    require(v.size() <= uint32(-1));
    v.dematerialize();
}
constexpr Tree::Tree (TreeObject v) :
    form(OBJECT), rep(in::REP_OBJECT), flags(0),
    length(v.size()), data{.as_object_ptr = v.data()}
{
    require(v.size() <= uint32(-1));
    v.dematerialize();
}
inline Tree::Tree (std::exception_ptr v) :
    form(ERROR), rep(in::REP_ERROR), flags(0), length(1), data{}
{
    auto e = SharedArray<std::exception_ptr>(1, move(v));
    const_cast<const std::exception_ptr*&>(data.as_error_ptr) = e.data();
    e.dematerialize();
}
constexpr Tree::Tree (Tree&& o) :
    form(o.form), rep(o.rep), flags(o.flags), length(o.length), data(o.data)
{
    const_cast<TreeForm&>(o.form) = UNDEFINED;
    const_cast<int8&>(o.rep) = 0;
    o.flags = 0;
    const_cast<uint32&>(o.length) = 0;
    const_cast<int64&>(o.data.as_int64) = 0;
}
constexpr Tree::Tree (const Tree& o) :
    form(o.form), rep(o.rep), flags(o.flags), length(o.length), data(o.data)
{
    if (rep < 0 && data.as_char_ptr) {
        ++ArrayOwnedHeader::get(data.as_char_ptr)->ref_count;
    }
}

constexpr Tree::~Tree () {
    if (rep < 0 && data.as_char_ptr) {
        auto header = ArrayOwnedHeader::get(data.as_char_ptr);
        if (header->ref_count) --header->ref_count;
        else in::delete_Tree_data(*this);
    }
}

constexpr Tree::operator Null () const {
    if (rep != in::REP_NULL) in::bad_Tree_form(*this, NULLFORM);
    return null;
}
constexpr Tree::operator bool () const {
    if (rep != in::REP_BOOL) in::bad_Tree_form(*this, BOOL);
    return data.as_bool;
}
constexpr Tree::operator char () const {
    switch (rep) { \
        case in::REP_STATICSTRING:
        case in::REP_SHAREDSTRING:
            if (length != 1) throw X<CantRepresent>("char", *this);
            return data.as_char_ptr[0];
        default: in::bad_Tree_form(*this, STRING);
    }
}
#define AYU_INTEGRAL_CONVERSION(T) \
constexpr Tree::operator T () const { \
    switch (rep) { \
        case in::REP_INT64: { \
            int64 v = data.as_int64; \
            if (int64(T(v)) == v) return v; \
            else throw X<CantRepresent>(#T, *this); \
        } \
        case in::REP_DOUBLE: { \
            double v = data.as_double; \
            if (double(T(v)) == v) return v; \
            else throw X<CantRepresent>(#T, *this); \
        } \
        default: in::bad_Tree_form(*this, NUMBER); \
    } \
}
AYU_INTEGRAL_CONVERSION(int8)
AYU_INTEGRAL_CONVERSION(uint8)
AYU_INTEGRAL_CONVERSION(int16)
AYU_INTEGRAL_CONVERSION(uint16)
AYU_INTEGRAL_CONVERSION(int32)
AYU_INTEGRAL_CONVERSION(uint32)
AYU_INTEGRAL_CONVERSION(int64)
AYU_INTEGRAL_CONVERSION(uint64)
#undef AYU_INTEGRAL_CONVERSION
constexpr Tree::operator double () const {
    switch (rep) {
         // Special case: allow null to represent +nan for JSON compatibility
        case in::REP_NULL: return +uni::nan;
        case in::REP_INT64: return data.as_int64;
        case in::REP_DOUBLE: return data.as_double;
        default: in::bad_Tree_form(*this, NUMBER);
    }
}
constexpr Tree::operator Str () const {
    switch (rep) {
        case in::REP_STATICSTRING:
        case in::REP_SHAREDSTRING:
            return Str(data.as_char_ptr, length);
        default: in::bad_Tree_form(*this, STRING);
    }
}
constexpr Tree::operator AnyString () const& {
    switch (rep) {
        case in::REP_STATICSTRING:
            return StaticString::Static(data.as_char_ptr, length);
        case in::REP_SHAREDSTRING: {
            if (data.as_char_ptr) {
                ++ArrayOwnedHeader::get(data.as_char_ptr)->ref_count;
            }
            return SharedString::Materialize(
                const_cast<char*>(data.as_char_ptr), length
            );
        }
        default: in::bad_Tree_form(*this, STRING);
    }
}
inline Tree::operator AnyString () && {
    switch (rep) {
        case in::REP_STATICSTRING:
            return StaticString::Static(data.as_char_ptr, length);
        case in::REP_SHAREDSTRING: {
            auto r = SharedString::Materialize(
                const_cast<char*>(data.as_char_ptr), length
            );
            new (this) Tree();
            return r;
        }
        default: in::bad_Tree_form(*this, STRING);
    }
}
inline Tree::operator UniqueString16 () const {
    return to_utf16(Str(*this));
}
constexpr Tree::operator TreeArraySlice () const {
    if (rep != in::REP_ARRAY) in::bad_Tree_form(*this, ARRAY);
    return TreeArraySlice(data.as_array_ptr, length);
}
constexpr Tree::operator TreeArray () const& {
    if (rep != in::REP_ARRAY) in::bad_Tree_form(*this, ARRAY);
    if (data.as_array_ptr) {
        ++ArrayOwnedHeader::get(data.as_array_ptr)->ref_count;
    }
    return TreeArray::Materialize(
        const_cast<Tree*>(data.as_array_ptr), length
    );
}
inline Tree::operator TreeArray () && {
    if (rep != in::REP_ARRAY) in::bad_Tree_form(*this, ARRAY);
    auto r = TreeArray::Materialize(
        const_cast<Tree*>(data.as_array_ptr), length
    );
    new (this) Tree();
    return r;
}
constexpr Tree::operator TreeObjectSlice () const {
    if (rep != in::REP_OBJECT) in::bad_Tree_form(*this, OBJECT);
    return TreeObjectSlice(data.as_object_ptr, length);
}
constexpr Tree::operator TreeObject () const& {
    if (rep != in::REP_OBJECT) in::bad_Tree_form(*this, OBJECT);
    if (data.as_object_ptr) {
        ++ArrayOwnedHeader::get(data.as_object_ptr)->ref_count;
    }
    return TreeObject::Materialize(
        const_cast<TreePair*>(data.as_object_ptr), length
    );
}
inline Tree::operator TreeObject () && {
    if (rep != in::REP_OBJECT) in::bad_Tree_form(*this, OBJECT);
    auto r = TreeObject::Materialize(
        const_cast<TreePair*>(data.as_object_ptr), length
    );
    new (this) Tree();
    return r;
}
inline Tree::operator std::exception_ptr () const {
    if (rep != in::REP_ERROR) in::bad_Tree_form(*this, ERROR);
    return *data.as_error_ptr;
}

constexpr const Tree* Tree::attr (Str key) const {
    if (rep != in::REP_OBJECT) in::bad_Tree_form(*this, OBJECT);
    for (auto& p : TreeObjectSlice(*this)) {
        if (p.first == key) return &p.second;
    }
    return null;
}
constexpr const Tree* Tree::elem (usize index) const {
    if (rep != in::REP_ARRAY) in::bad_Tree_form(*this, ARRAY);
    auto a = TreeArraySlice(*this);
    if (index < a.size()) return &a[index];
    else return null;
}
constexpr const Tree& Tree::operator[] (Str key) const {
    if (const Tree* r = attr(key)) return *r;
    else throw X<GenericError>(cat(
        "This tree has no attr with key \"", key, '"'
    ));
}
constexpr const Tree& Tree::operator[] (usize index) const {
    if (const Tree* r = elem(index)) return *r;
    else throw X<GenericError>(cat(
        "This tree has no elem with index ", index
    ));
}

} // ayu
