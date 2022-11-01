#pragma once

#include "common.h"
#include "type.h"

namespace ayu {

// This is a type storing dynamic values with optional names, intended to be the
// top-level item of a file.  Has fast insertion of newly-created unnamed items
// (usually one allocation including the new item).
//
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
     // Tried to create a document item with an illegal name.
    struct DocumentInvalidName : LogicError {
        String name;
        DocumentInvalidName (const String& n) : name(n) { }
    };
     // Tried to create a document item with a name that's already in use in
     // this document.
    struct DocumentDuplicateName : LogicError {
        String name;
        DocumentDuplicateName (const String& n) : name(n) { }
    };
     // Tried to delete a document item, but the wrong type was given during
     // deletion.
    struct DocumentDeleteWrongType : LogicError {
        Type existing;
        Type deleted_as;
        DocumentDeleteWrongType (Type e, Type d) :
            existing(e), deleted_as(d)
        { }
    };
     // (Debug only) Tried to delete a document item by pointer, but the given
     // pointer doesn't belong to this document.
    struct DocumentDeleteNotOwned : DebugError { };
     // Tried to delete a document item by name, but the given name isn't in
     // this document.
    struct DocumentDeleteMissing : LogicError {
        String name;
        DocumentDeleteMissing (const String& n) : name(n) { }
    };
}

} // namespace ayu
