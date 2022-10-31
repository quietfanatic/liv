
#pragma once

#include "common.h"
#include "type.h"

namespace ayu {

// This is a type storing dynamic values with optional names, intended to be the
//  top-level item of a file.  Has fast insertion of newly-created unnamed items
//  (usually one allocation including the new item).
// Keys starting with _ are reserved.
struct Document {
    in::DocumentData* data;

    Document ();
     // Deletes all items
    ~Document ();
    Document (const Document&) = delete;

    template <class T, class... Args>
    T* new_ (Args&&... args) {
        void* p = allocate(Type::CppType<T>());
        try {
            return new (p) T {std::forward<Args...>(args...)};
        }
        catch (...) {
            deallocate(p);
            throw;
        }
    }

     // This may be linear over the number of items in the document.
    template <class T, class... Args>
    T* new_named (const String& name, Args&&... args) {
        void* p = allocate_named(Type::CppType<T>(), name);
        try {
            return new (p) T (std::forward<Args...>(args...));
        }
        catch (...) {
            deallocate(p);
            throw;
        }
    }

     // Throws X::DocumentDeleteWrongType if T is not the type of *p
    template <class T>
    void delete_ (T* p) {
        delete_(Type::CppType<T>(), (Mu*)p);
    }

    void* allocate (Type);
    void* allocate_named (Type, const String&);
    void delete_ (Type, Mu*);
    void delete_named (const String&);

    void deallocate (void* p);
};

namespace X {
    struct DocumentInvalidName : LogicError {
        String name;
        DocumentInvalidName (const String& n) : name(n) { }
    };
    struct DocumentDuplicateName : LogicError {
        String name;
        DocumentDuplicateName (const String& n) : name(n) { }
    };
    struct DocumentDeleteWrongType : LogicError {
        Type existing;
        Type deleted_as;
        DocumentDeleteWrongType (Type e, Type d) : existing(e), deleted_as(d) { }
    };
    struct DocumentDeleteNotOwned : DebugError { };
    struct DocumentDeleteMissing : LogicError {
        String name;
        DocumentDeleteMissing (const String& n) : name(n) { }
    };
}

} // namespace ayu
