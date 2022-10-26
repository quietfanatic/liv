#pragma once

#include <type_traits>

#include "accessors.h"
#include "dynamic.h"

namespace hacc {

 // Represents a dynamically typed object with reference semantics.
 //
 // Can reference any object that can be accessed through an Accessor, even if
 // its address cannot be taken.  The empty value will cause null derefs if
 // you do anything with it.  Just as with C++ references or pointers, do not
 // dereference a Reference after the object it refers to goes away.
 //
 // The Reference object itself is immutable once created.
 //
 // This class contains convenience methods that call serialize functions on
 // the referred-to object.
struct Reference {
    Mu* const host;
    const in::AccessorOrType aot;

    constexpr Reference () : host(null), aot(null) { }
    Reference (Mu* h, const in::Accessor* a) : host(h), aot(a) { }
     // Construct from type and abstract pointer.  Used by serialize.
    Reference (Type t, Mu* p) : host(p), aot(t) { }
     // Construct from a pointer.
    template <class T>
    Reference (T* p) :
        host(reinterpret_cast<Mu*>(p)),
        aot(Type::CppType<T>())
    { }
     // Construct from a const pointer.  Makes a readonly reference.
    template <class T>
    Reference (const T* p) :
        host(reinterpret_cast<Mu*>(const_cast<T*>(p))),
        aot(Type::CppType<T>(), true)
    { }
     // Construct from a Dynamic.
    Reference (Dynamic& d) : host(d.data), aot(d.type) { }
     // For use in attr_func and elem_func.
    template <class From, class Acr,
        std::enable_if_t<std::is_same_v<typename Acr::AccessorFromType, From>, bool> = true
    >
    Reference (From& h, Acr&& a) :
        host(reinterpret_cast<Mu*>(&h)),
        aot(static_cast<const in::Accessor*>(new Acr(a)))
    { }

    Reference (const Reference& o) :
        host(o.host),
        aot(o.aot)
    { }
    Reference (Reference&& o) :
        host(o.host),
        aot(std::move(const_cast<in::AccessorOrType&>(o.aot)))
    {
        const_cast<Mu*&>(o.host) = null;
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
     //  still has a type, and calling type() or address() may be valid
     //  depending on the accessor type.  An empty Reference has no type and
     //  no operations on it are valid (besides these of course).
    bool empty () const { return !aot; }
    explicit operator bool () const { return host; }

    bool readonly () const { return aot.readonly(); }
    void require_writable () const;
    Type type () const { return aot.type(*host); }
    Mu* address () const { return aot.address(*host); }
     // Can throw X::CannotCoerce, even if the result is null.
    Mu* address_as (Type t) const { return type().cast_to(t, address()); } 
     // Will throw X::UnaddressableReference if this Reference is not empty but
     //  the return of address() is null.
    Mu* require_address () const;
     // Can throw either of the above.
    Mu* require_address_as (Type t) const { return type().cast_to(t, require_address()); }
    void read (Callback<void(const Mu&)> cb) const {
        access(in::ACR_READ, reinterpret_cast<Callback<void(Mu&)>&>(cb));
    }
    void read_as (Type t, Callback<void(const Mu&)> cb) const {
        read([&](const Mu& v){
            const Mu& tv = *type().cast_to(t, &v);
            cb(tv);
        });
    }
    void write (Callback<void(Mu&)> cb) const { access(in::ACR_WRITE, cb); }
    void write_as (Type t, Callback<void(Mu&)> cb) const {
        write([&](Mu& v){
            Mu& tv = *type().cast_to(t, &v);
            cb(tv);
        });
    }
    void modify (Callback<void(Mu&)> cb) const { access(in::ACR_MODIFY, cb); }
    void modify_as (Type t, Callback<void(Mu&)> cb) const {
        write([&](Mu& v){
            Mu& tv = *type().cast_to(t, &v);
            cb(tv);
        });
    }

    template <class T>
    T* address_as () const {
        return (T*)address_as(Type::CppType<T>());
    }
    template <class T>
    T* require_address_as () const {
        return (T*)require_address_as(Type::CppType<T>());
    }
    template <class T>
    void read_as (Callback<void(const T&)> cb) const {
        read_as(Type::CppType<T>(), reinterpret_cast<Callback<void(const Mu&)>&>(cb));
    }
    template <class T>
    void write_as (Callback<void(T&)> cb) const {
        write_as(Type::CppType<T>(), reinterpret_cast<Callback<void(Mu&)>&>(cb));
    }
    template <class T>
    void modify_as (Callback<void(T&)> cb) const {
        modify_as(Type::CppType<T>(), reinterpret_cast<Callback<void(Mu&)>&>(cb));
    }

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
    template <class T>
    void set_as (const T& new_v) {
        if (auto a = address_as<T>()) *a = new_v;
        else write([&](Mu& v){
            reinterpret_cast<T&>(v) = new_v;
        });
    }

     // Shortcuts for serialize functions
    Tree to_tree () const;
    void from_tree (Tree t) const;
     // If this reference is got through value_funcs or somesuch, then calling
     //  these a bunch of times may be slow.
    std::vector<String> get_keys () const;
    void set_keys (const std::vector<String>& ks) const;
    Reference maybe_attr (Str key) const;
    Reference attr (Str key) const;
    usize get_length () const;
    void set_length (usize l) const;
    Reference maybe_elem (usize index) const;
    Reference elem (usize index) const;

     // These are used by serialize.  They will be most efficient if this
     //  Reference has an address().
    Reference chain (const in::Accessor*) const;
    Reference chain_attr_func (Reference(*)(Mu&, Str), Str) const;
    Reference chain_elem_func (Reference(*)(Mu&, usize), usize) const;
     // Kinda internal, TODO move to internal namespace
    void access (in::AccessOp op, Callback<void(Mu&)> cb) const {
        if (op != in::ACR_READ) require_writable();
        aot.access(op, *host, cb);
    }

    Reference operator [] (Str key) const { return attr(key); }
    Reference operator [] (usize index) const { return elem(index); }

    template <class T>
    operator const T* () const { return require_address_as<T>(); }
    template <class T>
    operator T* () const {
        require_writable();
        return require_address_as<T>();
    }
};

 // Reference comparison is best-effort.  References compare equal if:
 //  1. They have the same host and accessor pointers, or
 //  2. They have the same type, they both have an address(), and those
 //      addresses are equal.
 // This means that unaddressable references constructed through attr_func or
 //  elem_func will not be comparable, and thus cannot be serialized.
static bool operator == (const Reference& a, const Reference& b) {
    if (a.host == b.host && a.aot == b.aot) return true;
    if (!a || !b) return false;
    if (a.type() != b.type()) return false;
    auto aa = a.address();
    return aa && aa == b.address();
}
static bool operator != (const Reference& a, const Reference& b) {
    return !(a == b);
}

namespace X {
     // TODO: change to contain Path
    struct WriteReadonlyReference : LogicError {
        Reference ref;
        WriteReadonlyReference (const Reference& r) : ref(r) { }
    };
    struct UnaddressableReference : LogicError {
        Reference ref;
        UnaddressableReference (const Reference& r) : ref(r) { }
    };
}

} // namespace hacc

 // Allow Reference to be a key in unordered_map
namespace std {
    template <>
    struct hash<hacc::Reference> {
        size_t operator () (const hacc::Reference& r) const {
             // This is in a different order than operator==, but
             //  I don't think that should be a problem, assuming the
             //  address is deterministic.
            auto a = r.address();
            if (a) return hacc::in::hash_combine(
                hash<void*>()((void*)a),
                hash<hacc::Type>()(r.type())
            );
            else return hacc::in::hash_combine(
                hash<void*>()((void*)r.host),
                hash<void*>()((void*)r.aot.data)
            );
        }
    };
}

 // Break cyclic dependency
 // TODO: this is no longer necessary
#include "serialize.h"
namespace hacc {

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

}
