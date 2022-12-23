#pragma once

#include "internal/common-internal.h"
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
    T* new_named (Str name, Args&&... args) {
        void* p = allocate_named(Type::CppType<T>(), name);
        try {
            return new (p) T (std::forward<Args...>(args...));
        }
        catch (...) {
            deallocate(p);
            throw;
        }
    }

     // Throws DocumentDeleteWrongType if T is not the type of *p.
     // In debug mode, verifies that the given object actually belongs to this
     // Document.
    template <class T>
    void delete_ (T* p) {
        delete_(Type::CppType<T>(), (Mu*)p);
    }

    void* allocate (Type);
    void* allocate_named (Type, Str);
    void delete_ (Type, Mu*);
    void delete_named (Str);

    void deallocate (void* p);
};

 // General category of errors coming from ayu::Document
 // TODO: Add a Location to this
struct DocumentError : Error { };
 // Tried to create a document item with an illegal name.
struct DocumentInvalidName : DocumentError {
    std::string name;
};
 // Tried to create a document item with a name that's already in use in
 // this document.
struct DocumentDuplicateName : DocumentError {
    std::string name;
};
 // Tried to delete a document item, but the wrong type was given during
 // deletion.
struct DocumentDeleteWrongType : DocumentError {
    Type existing;
    Type deleted_as;
};
 // Tried to delete a document item by name, but the given name isn't in
 // this document.
struct DocumentDeleteMissing : DocumentError {
    std::string name;
};

} // namespace ayu
