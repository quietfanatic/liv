#pragma once

#include <source_location>
#include "common.h"
#include "pointer.h"

namespace ayu {

// Because C++ doesn't allow aggregate initialization for classes with virtual
// methods, anything that inherits from std::exception must have a user-defined
// constructor.  Yuck!  So we use multiple inheritance to get around that.

struct ExceptionBase : std::exception {
     // Gotta cache the generated error message or the exception handling
     // system will reference stack garbage.
    mutable std::string mess_cache;
     // Calls item_to_string on whatever the error type is.
    const char* what () const noexcept final;
    virtual Pointer ptr () const noexcept = 0;
};

template <class Err>
struct X : ExceptionBase, Err {
     // TODO: This gets the source location of the constructor, not the source
     // of the throw, add another function raise<>()
    std::source_location loc = std::source_location::current();
    Pointer ptr () const noexcept final {
        return static_cast<const Err*>(this);
    }
     // This has to be {} and not () to allow flattening aggregate
     // initialization, but this also means we get narrowing conversion warnings
     // for integers.  Putting another {} around the argument suppresses that,
     // but it makes error messages worse when conversion fails.
    template <class... Args>
    constexpr explicit X(Args&&... args) :
        Err{&loc, std::forward<Args>(args)...}
    { }
     // No sense optimizing exceptions for speed, so optimize for size instead
    [[gnu::cold]]
    ~X () { }
};

namespace in {

 // Called when an exception is thrown in a place where the library can't
 // properly clean up after itself, such as when a resource value throws
 // from its destructor.
[[noreturn]] void unrecoverable_exception (std::exception& e, Str when);

} // namespace in

} // namespace ayu
