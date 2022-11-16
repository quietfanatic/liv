#pragma once

#include <cassert>
#include <type_traits>

#include "internal/accessors-internal.h"
#include "internal/descriptors-internal.h"
#include "dynamic.h"
#include "location.h"

namespace ayu {

 // Represents a dynamically typed object with reference semantics.
 //
 // Can reference any object that can be accessed through an Accessor, even if
 // its address cannot be taken.  Just as with C++ references or pointers, do
 // not dereference a Reference after the object it refers to goes away.
 //
 // This Reference object itself is immutable once created.
struct Reference {
    Mu* const host;
    const in::Accessor* const acr;

     // The empty value will cause null derefs if you do anything with it.
    constexpr Reference () : host(null), acr(null) { }
     // Construct from internal data.  Does not take ownership (host is a
     // reference-like pointer and acr is refcounted).
    Reference (Mu* h, const in::Accessor* a) : host(h), acr(a) {
        if (acr) acr->inc();
    }
     // Construct from type and abstract pointer.  Used by serialize.
    Reference (Type t, Mu* p) : Reference(p, &t.desc->identity_acr) { }
     // Construct from a pointer.
    template <class T>
    Reference (T* p) : Reference(
        reinterpret_cast<Mu*>(p),
        &Type::CppType<T>().desc->identity_acr
    ) { }
     // Construct from a const pointer.  Makes a readonly reference.
    template <class T>
    Reference (const T* p) : Reference(
        reinterpret_cast<Mu*>(const_cast<T*>(p)),
        &Type::CppType<T>().desc->readonly_identity_acr
    ) { }
     // Construct from a Dynamic.
     // TODO: construct readonly Reference from const Dynamic?
    Reference (Dynamic& d) : Reference(d.data, &d.type.desc->identity_acr) { }
     // For use in attr_func and elem_func.
     // Todo: Also check std::is_base_of
    template <class From, class Acr,
        std::enable_if_t<
            std::is_same_v<typename Acr::AccessorFromType, From>, bool
        > = true
    >
    Reference (From& h, Acr&& a) : Reference(
        reinterpret_cast<Mu*>(&h), new Acr(a)
    ) { }
     // Copy and move construction and assignment
    Reference (const Reference& o) : Reference(o.host, o.acr) { }
    Reference (Reference&& o) :
        host(o.host), acr(o.acr)
    {
        const_cast<Mu*&>(o.host) = null;
        const_cast<const in::Accessor*&>(o.acr) = null;
    }
    Reference& operator = (const Reference& o) {
        this->~Reference();
        new (this) Reference(o);
        return *this;
    }
    Reference& operator = (Reference&& o) {
        this->~Reference();
        new (this) Reference(std::move(o));
        return *this;
    }

     // These are slightly different semantically.  A null (false) Reference
     // still has a type, and calling type() or address() may be valid
     // depending on the accessor type.  An empty Reference has no type and
     // no operations on it are valid (besides these of course).
    bool empty () const { assert(!!host == !!acr); return !acr; }
    explicit operator bool () const { return host; }
     // Writing to this reference throws if this is true
    bool readonly () const { return acr->accessor_flags & in::ACR_READONLY; }
     // Throws X::WriteReadonlyReference if readonly()
    void require_writable () const;
     // Get type of referred-to item
    Type type () const { return acr->type(*host); }

     // Returns null if this reference is not addressable.
    Mu* address () const { return acr->address(*host); }
     // Can throw X::CannotCoerce, even if the result is null.
    Mu* address_as (Type t) const { return type().cast_to(t, address()); } 
    template <class T>
    T* address_as () const {
        return (T*)address_as(Type::CppType<T>());
    }
     // Will throw X::UnaddressableReference if this Reference is not empty but
     // the return of address() is null.
    Mu* require_address () const;
     // Can throw either X::CannotCoerce or X::UnaddressableReference
    Mu* require_address_as (Type t) const { return type().cast_to(t, require_address()); }
    template <class T>
    T* require_address_as () const {
        return (T*)require_address_as(Type::CppType<T>());
    }

     // Read with callback
    void read (Callback<void(const Mu&)> cb) const {
        access(in::ACR_READ, reinterpret_cast<Callback<void(Mu&)>&>(cb));
    }
     // Cast and read with callback
    void read_as (Type t, Callback<void(const Mu&)> cb) const {
        read([&](const Mu& v){
            const Mu& tv = *type().cast_to(t, &v);
            cb(tv);
        });
    }
    template <class T>
    void read_as (Callback<void(const T&)> cb) const {
        read_as(Type::CppType<T>(), reinterpret_cast<Callback<void(const Mu&)>&>(cb));
    }
     // Write with callback
    void write (Callback<void(Mu&)> cb) const { access(in::ACR_WRITE, cb); }
     // Cast and write with callback
    void write_as (Type t, Callback<void(Mu&)> cb) const {
        write([&](Mu& v){
            Mu& tv = *type().cast_to(t, &v);
            cb(tv);
        });
    }
    template <class T>
    void write_as (Callback<void(T&)> cb) const {
        write_as(Type::CppType<T>(), reinterpret_cast<Callback<void(Mu&)>&>(cb));
    }
     // Modify in-place with callback
    void modify (Callback<void(Mu&)> cb) const { access(in::ACR_MODIFY, cb); }
     // Cast and modify in-place with callback
    void modify_as (Type t, Callback<void(Mu&)> cb) const {
        write([&](Mu& v){
            Mu& tv = *type().cast_to(t, &v);
            cb(tv);
        });
    }
    template <class T>
    void modify_as (Callback<void(T&)> cb) const {
        modify_as(Type::CppType<T>(), reinterpret_cast<Callback<void(Mu&)>&>(cb));
    }

     // Copying getter.
     // TODO: This doesn't cast properly
    template <class T>
    T get_as () const {
        if (auto a = address_as<T>()) return *a;
        else {
            T r;
            read([&](const Mu& v){
                r = reinterpret_cast<const T&>(v);
            });
            return r;
        }
    }
     // Assign to the referenced item with const ref.
     // TODO: This doesn't cast properly
    template <class T>
    void set_as (const T& new_v) {
        if (auto a = address_as<T>()) *a = new_v;
        else write([&](Mu& v){
            reinterpret_cast<T&>(v) = new_v;
        });
    }

     // Shortcuts for casting functions from type.h
    Reference try_upcast_to (Type t) {
        return Reference(t, acr->type(*host).try_upcast_to(t, address()));
    }
    Reference upcast_to (Type t) {
        return Reference(t, acr->type(*host).upcast_to(t, require_address()));
    }
    Reference try_downcast_to (Type t) {
        return Reference(t, acr->type(*host).try_downcast_to(t, address()));
    }
    Reference downcast_to (Type t) {
        return Reference(t, acr->type(*host).downcast_to(t, require_address()));
    }
    Reference try_cast_to (Type t) {
        return Reference(t, acr->type(*host).try_cast_to(t, address()));
    }
    Reference cast_to (Type t) {
        return Reference(t, acr->type(*host).cast_to(t, require_address()));
    }

     // Shortcuts for serialize functions
    Tree to_tree () const;
    void from_tree (Tree t) const;
     // If this reference is got through value_funcs or somesuch, then calling
     // these a bunch of times may be slow.
    std::vector<String> get_keys () const;
    void set_keys (const std::vector<String>& ks) const;
    Reference maybe_attr (Str key) const;
    Reference attr (Str key) const;
    usize get_length () const;
    void set_length (usize l) const;
    Reference maybe_elem (usize index) const;
    Reference elem (usize index) const;

     // These are used by serialize.  They will be most efficient if this
     // Reference has an address().
    Reference chain (const in::Accessor*) const;
    Reference chain_attr_func (Reference(*)(Mu&, Str), Str) const;
    Reference chain_elem_func (Reference(*)(Mu&, usize), usize) const;
     // Kinda internal, TODO move to internal namespace
    void access (in::AccessOp op, Callback<void(Mu&)> cb) const {
        if (op != in::ACR_READ) require_writable();
        acr->access(op, *host, cb);
    }

     // Syntax sugar.
    Reference operator [] (Str key) const { return attr(key); }
    Reference operator [] (usize index) const { return elem(index); }
    template <class T>
    operator const T* () const { return require_address_as<T>(); }
    template <class T>
    operator T* () const {
        require_writable();
        return require_address_as<T>();
    }

    ~Reference () { if (acr) acr->dec(); }
};

 // Reference comparison is best-effort.  References compare equal if:
 //  1. They have the same host and accessor pointers, or
 //  2. They have the same type, they both have an address(), and those
 //     addresses are equal.
 // This means that unaddressable references constructed through attr_func or
 // elem_func will not be comparable, and thus cannot be serialized.  Those
 // references are likely to be very inefficient anyway, so try not to create
 // them.
 // TODO: Should this return false if only one Reference is readonly?
inline bool operator == (const Reference& a, const Reference& b) {
    if (a.host == b.host && a.acr == b.acr) return true;
    if (!a || !b) return false;
    if (a.type() != b.type()) return false;
    auto aa = a.address();
    return aa && aa == b.address();
}
inline bool operator != (const Reference& a, const Reference& b) {
    return !(a == b);
}

namespace X {
     // Tried to write to a readonly reference.
    struct WriteReadonlyReference : LogicError {
        Location location;
        WriteReadonlyReference (const Reference& r);
    };
     // Used the reference in a context where its address was required, but it
     // has no address.
    struct UnaddressableReference : LogicError {
        Location location;
        UnaddressableReference (const Reference& r);
    };
}

} // namespace ayu

 // Allow Reference to be a key in unordered_map
template <>
struct std::hash<ayu::Reference> {
    size_t operator () (const ayu::Reference& r) const {
         // This is in a different order than operator==, but I don't think
         // that should be a problem, assuming the address is deterministic.
        auto a = r.address();
        if (a) return ayu::in::hash_combine(
            hash<void*>()((void*)a),
            hash<ayu::Type>()(r.type())
        );
        else return ayu::in::hash_combine(
            hash<void*>()((void*)r.host),
            hash<void*>()((void*)r.acr)
        );
    }
};

 // Break cyclic dependency
 // TODO: Check if this is still necessary
#include "serialize.h"
namespace ayu {

inline Tree Reference::to_tree () const { return item_to_tree(*this); }
inline void Reference::from_tree (Tree t) const { item_from_tree(*this, t); }
inline std::vector<String> Reference::get_keys () const { return item_get_keys(*this); }
inline void Reference::set_keys (const std::vector<String>& ks) const { item_set_keys(*this, ks); }
inline Reference Reference::maybe_attr (Str key) const { return item_maybe_attr(*this, key); }
inline Reference Reference::attr (Str key) const { return item_attr(*this, key); }
inline usize Reference::get_length () const { return item_get_length(*this); }
inline void Reference::set_length (usize l) const { item_set_length(*this, l); }
inline Reference Reference::maybe_elem (usize index) const { return item_maybe_elem(*this, index); }
inline Reference Reference::elem (usize index) const { return item_elem(*this, index); }

namespace X {
    inline WriteReadonlyReference::WriteReadonlyReference (const Reference& r) :
        location(reference_to_location(r))
    { }
    inline UnaddressableReference::UnaddressableReference (const Reference& r) :
        location(reference_to_location(r))
    { }
}

} // namespace ayu
