// A resource name is kind of like a budget IRI that only has a scheme and a
// path, and no percent-encoding.  They're represented as plain strings for now.
// At some point, we may start using real IRIs.
//
// scheme:/path/to/file.ayu

#pragma once

#include "common.h"
#include "compat.h"

namespace ayu {

///// RESOURCE NAMES

 // Does these things:
 //   - Replaces foo//bar with foo/bar
 //   - Replaces foo/./bar with foo/bar
 //   - Replaces foo/../bar with bar
 // Throws X::InvalidResourceName if there are any invalid characters,
 // or if the path tries to escape the root (such as by starting with /..).
String canonicalize (Str name);

bool is_absolute (Str name);
inline bool is_relative (Str name) { return !is_absolute(name); }

 // Returns "" if there is no scheme.  "" is not a valid scheme.
//Str get_scheme (Str name);
 // Returns "" if there is no path.  "" is not a valid path.
//Str get_path (Str name);

 // Resolves the possibly-relative resource name into an absolute name.  If
 // name is already absolute, just returns it, otherwise makes it absolute by
 // attaching it to the contents of base up to the last /.  This does mean that
 // whether base ends with a / changes the result.
 //   resolves("foo", "bar/qux") == "bar/foo"
 //   resolves("foo", "bar/qux/") == "bar/qux/foo"
 // If base is unspecified, uses the current resource's name.  If there is no
 // current resource (and name is not already absolute) throws
 // X::UnresolvedResourceName
String resolve (Str name, Str base = Str());

///// RESOURCE SCHEMES

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

     // If you want to do some of your own validation besides the standard
     // checking for invalid characters and such.
    virtual bool is_valid_path (Str) const {
        return true;
    }
     // Turn a resource path into a filename.  If "" is returned, it means there
     // is no valid filename for this path.  It is okay to return non-existent
     // filenames.
    virtual String get_file (Str) const { return ""; }
     // TODO: Non-file resource schemes

    ResourceScheme (Str scheme_name, bool auto_activate = true) :
        scheme_name(scheme_name)
    {
        if (auto_activate) activate();
    }

    virtual ~ResourceScheme () {
        deactivate();
    }

     // These are called in the constructor (by default) and destructor, so you
     // don't have to call them yourself.
    void activate () const;
    void deactivate () const;
};

 // TEMPORARY FOR BACK COMPAT
void set_file_resource_root (Str root);
Str file_resource_root ();
String resource_filename (Str name);

 // Maps resource names to the contents of a folder.
struct FileResourceScheme : ResourceScheme {
    String folder;

    String get_file (Str path) const override {
        return folder + path;
    }

    FileResourceScheme (String scheme, String folder, bool auto_activate = true)
        : ResourceScheme(scheme, auto_activate), folder(folder)
    { }
};

namespace X {
    struct ResourceNameError : LogicError { };
     // Resource name contains invalid characters or something
    struct InvalidResourceName : ResourceNameError {
        String name;
        InvalidResourceName (String&& name) : name(std::move(name)) { }
    };
     // Resource name couldn't be resolved to an absolute name, such as if a
     // relative path was given but there's no current resource.
    struct UnresolvedResourceName : ResourceNameError {
        String name;
        UnresolvedResourceName (String&& name) : name(std::move(name)) { }
    };
     // Tried to use a resource name starting with /.. or equivalent.
    struct ResourceNameOutsideRoot : ResourceNameError {
        String name;
        ResourceNameOutsideRoot (String&& name) : name(std::move(name)) { }
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
