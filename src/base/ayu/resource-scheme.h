// A resource name is an IRI.  Interpretation of IRIs is determined by
// globally-registered ResourceScheme objects, but generally they refer to
// files on disk.
//     scheme:/path/to/file.ayu

#pragma once

#include "../iri/iri.h"
#include "common.h"
#include "compat.h"
#include "type.h"

namespace ayu {
using iri::IRI;

 // Registers a resource scheme at startup.  The path parameter passed to all
 // the virtual methods is just the path part of the name, and is always
 // canonicalized and absolute.
 //
 // Currently, resources from a scheme are only allowed to reference other
 // resources from the same scheme.
 //
 // If no ResourceSchemes are active, then a default resource scheme with the
 // name "file" will be used, which maps resource names to files on disk.
struct ResourceScheme {
     // Must be a valid scheme name matching [a-z][a-z0-9+.-]*
    const String scheme_name;

     // If you want to do some of your own validation besides the standard IRI
     // validation.  If this returns false, X::UnacceptableResourceName will
     // be thrown.  The provided IRI will not have a fragment.
    virtual bool accepts_iri (const IRI& iri) const {
        return !!iri;
    }
     // If you want to limit the allowed top-level types of your resources.
     // This is called when load(), reload(), save(), or set_value() is called
     // on a resource of this scheme, or a resource of this scheme is
     // constructed with a specific provided value.  If this returns false,
     // X::UnacceptableResourceType will be thrown.
    virtual bool accepts_type (Type) const {
        return true;
    }
     // Turn an IRI into a filename.  If "" is returned, it means there is no
     // valid filename for this IRI.  It is okay to return non-existent
     // filenames.
    virtual String get_file (const IRI&) const { return ""; }
     // TODO: Non-file resource schemes

    explicit ResourceScheme (Str scheme_name, bool auto_activate = true) :
        scheme_name(scheme_name)
    {
        if (auto_activate) activate();
    }

    ResourceScheme (const ResourceScheme&) = delete;
    ResourceScheme (ResourceScheme&& o) = delete;
    ResourceScheme& operator = (const ResourceScheme&) = delete;
    ResourceScheme& operator = (ResourceScheme&&) = delete;

    virtual ~ResourceScheme () {
        if (!scheme_name.empty()) deactivate();
    }

     // These are called in the constructor (by default) and destructor, so you
     // don't have to call them yourself.
    void activate () const;
    void deactivate () const;
};

 // Maps resource names to the contents of a folder.
struct FileResourceScheme : ResourceScheme {
    String folder;

    bool accepts_iri (const IRI& iri) const override {
        return iri && !iri.has_authority() && !iri.has_query()
            && iri.is_hierarchical();
    }

    String get_file (const IRI& iri) const override {
        return folder + iri::decode(iri.path());
    }

    FileResourceScheme (String scheme, String folder, bool auto_activate = true)
        : ResourceScheme(scheme, auto_activate), folder(folder)
    { }
};

namespace X {
    struct ResourceNameError : LogicError { };
     // An invalid IRI was given as a resource name.
    struct InvalidResourceName : ResourceNameError {
        String name;
        InvalidResourceName (String&& name) : name(std::move(name)) { }
    };
     // Tried to use an IRI as a resource name but its scheme was not registered
    struct UnknownResourceScheme : ResourceNameError {
        String name;
        UnknownResourceScheme (String&& name) : name(std::move(name)) { }
    };
     // A valid IRI was given but its ResourceScheme didn't like it.
    struct UnacceptableResourceName : ResourceNameError {
        String name;
        UnacceptableResourceName (String&& name) : name(std::move(name)) { }
    };
     // Tried to load or set_value a resource with a type that the
     // ResourceScheme didn't accept.
    struct UnacceptableResourceType : ResourceNameError {
        String name;
        Type type;
        UnacceptableResourceType (String&& name, Type t) :
            name(std::move(name)), type(t)
        { }
    };
     // Tried to register a ResourceScheme with an invalid name.
    struct InvalidResourceScheme : ResourceNameError {
        String scheme;
        InvalidResourceScheme (String&& scheme) : scheme(std::move(scheme)) { }
    };
     // Tried to register multiple ResourceSchemes with the same name.
    struct DuplicateResourceScheme : ResourceNameError {
        String scheme;
        DuplicateResourceScheme (String&& scheme) : scheme(std::move(scheme)) { }
    };
}

} // namespace ayu
