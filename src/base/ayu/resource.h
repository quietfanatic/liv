 // A resource represents a top-level named piece of program data.  A
 // resource has:
 //     - a source, which is by default a file on disk
 //     - a name, which in the case of files, is essentially its file path
 //     - a value, which is a Dynamic
 //     - a state, which is usually UNLOADED or LOADED.
 // Resources can be loaded, reloaded, unloaded, and saved.
 //
 // Resource names may not contain :, ?, or # (these are reserved for URIs)
 //
 // Resources can have no name, in which case they are anonymous.  Anonymous
 // resources cannot be reloaded or saved, but they can be unloaded.  Anonymous
 // resources can contain references to named resources, and those references
 // will be updated if those resources are reloaded.  Named resources cannot
 // be saved if they contain references to anonymous resources, because there's
 // no way to serialize that reference as a path.
 //
 // So, if you have global variables that reference things in resources, make
 // those global variables anonymous resources, and they will be automatically
 // updated whenever the resource is reloaded.

#pragma once

#include "common.h"
#include "path.h"
#include "reference.h"

namespace ayu {

///// RESOURCES

enum ResourceState {
     // The resource is not loaded and has an empty value.
    UNLOADED,
     // This resource is fully loaded and has a non-empty up-to-date value,
     // though that value may not reflect what is on disk.
    LOADED,

     // The following states will only be encountered while an ayu resource
     // operation is ongoing.

     // load() is being called on this resource.  Its value may be partially
     // constructed.
    LOAD_CONSTRUCTING,
     // load() is being called on this resource, but there was an error, so
     // its destructor is being or will be called.
    LOAD_ROLLBACK,
     // save() is being called on this resource, but it is not being written to
     // disk yet.
    SAVE_VERIFYING,
     // save() is being called on this resource, and it is being or will be
     // written to disk.
    SAVE_COMMITTING,
     // unload() is being called on this resource, and other resources are being
     // scanned for references to it.
    UNLOAD_VERIFYING,
     // unload() is being called on this resource, and its destructor is being
     // or will be called.  There is no UNLOAD_ROLLBACK because unload doesn't
     // need to roll anything back.
    UNLOAD_COMMITTING,
     // reload() is being called on this resource, and its new value is being
     // constructed.  value() will return its (maybe incomplete) new value.
    RELOAD_CONSTRUCTING,
     // reload() is being called on this resource, and other resources are being
     // scanned for references to update.
    RELOAD_VERIFYING,
     // reload() is being called on this resource, but there was an error, so
     // its new value is being destructed and its old value will be restored.
    RELOAD_ROLLBACK,
     // reload() is being called on this resource, and its old value is being
     // destructed.
    RELOAD_COMMITTING,
};
 // Get the string name of a resource state.
Str show_ResourceState (ResourceState);

 // The Resource class refers to a resource with reference semantics.
 // This class itself is cheap to copy.
struct Resource {
     // Internal data is kept perpetually, but may be refcounted at some point
     // (it's about 7-10 words long)
    in::ResourceData* data;

    constexpr Resource (in::ResourceData* d = null) : data(d) { }
     // Refers to the resource with this name, but does not load it yet.  If
     // name is empty, gets the current resource if there is one, otherwise
     // throws InvalidResourceName.
     // TODO: Change current resource name to something like "$" or "_"
    Resource (Str name);
     // TODO: remove these, I think they're useless
    Resource (const char* name) : Resource(Str(name)) { }
    Resource (const String& name) : Resource(Str(name)) { }
     // Creates the resource already loaded with the given data, without reading
     // from disk.  Will throw if a resource with this name is already loaded.
    Resource (Str name, Dynamic&& value);
     // Creates an anonymous resource with no value
    Resource (Null);
     // Creates an anonymous resource with the given value
    Resource (Null, Dynamic&& value);

     // Returns the name in absolute form.
    Str name () const;
     // See enum ResourceState
    ResourceState state () const;

     // If the resource is UNLOADED, automatically loads the resource from disk.
     // Will throw if autoloading the resource but the file does not exist.
    Dynamic& value () const;
     // Gets the value without autoloading.  If the result is empty, do not
     // write to it.  Nothing bad will happen immediately, but the value will
     // be overwritten when the resource is loaded.
    Dynamic& get_value () const;
     // If the resource is UNLOADED, sets is state to LOADED without loading
     // from disk.  The returned value will be empty and you should write to
     // it.  Throws X::InvalidResourceState if the resource's state is
     // anything but UNLOADED, LOADED, or LOAD_CONSTRUCTING.
    Dynamic& set_value () const;

     // Automatically loads and returns a reference to the value, which can be
     // coerced to a pointer.
    Reference ref () const;
     // Gets a reference to the value without automatically loading.  If the
     // resource is UNLOADED, returns an empty Reference.
    Reference get_ref () const;

     // Syntax sugar
    explicit operator bool () { return data; }
    Reference operator [] (Str key) {
         // TODO: change to ref()[key]
        return Reference(value())[key];
    }
    Reference operator [] (usize index) {
        return Reference(value())[index];
    }
};

 // Resources are considered equal if their names are equal (Resources with the
 // same name will always have the same data pointer).
static bool operator == (Resource a, Resource b) { return a.data == b.data; }
static bool operator != (Resource a, Resource b) { return !(a == b); }

///// RESOURCE OPERATIONS

 // Loads a resource.  Does nothing if the resource is not UNLOADED.  Throws
 // if the file doesn't exist on disk or can't be opened.
void load (Resource);
 // Loads multiple resources at once.  If an exception is thrown, all the loads
 // will be cancelled and all of the given resources will end up in the UNLOADED
 // state (unless they were already LOADED beforehand).
void load (const std::vector<Resource>&);

 // Moves old_res's value to new_res.  Does not change the names of any Resource
 // objects, just the mapping from names to values.  Does not affect any files
 // on disk.  Will throw if old_res is not LOADED, or if new_res is not
 // UNLOADED.  After renaming, old_res will be UNLOADED and new_res will
 // be LOADED.
void rename (Resource old_res, Resource new_res);

 // Saves a loaded resource to disk.  Throws if the resource is not LOADED.  May
 // overwrite an existing file on disk.
void save (Resource);
 // Saves multiple resources at once.  This will be more efficient than saving
 // them one at a time (the universe only needs to be scanned once for
 // serializing references).  If an error is thrown, none of the resources will
 // be saved.  (Exception: there may be some cases where if an error is thrown
 // while writing to disk, the on-disk resources may be left in an inconsistent
 // state.)
void save (const std::vector<Resource>&);

 // Clears the value of the resource and sets its state to UNLOADED.  Does
 // nothing if the resource is UNLOADED, and throws if it is LOADING.  Scans all
 // other LOADED resources to make sure none of them are referencing this
 // resource, and if any are, this call will throw X::UnloadWouldBreak and the
 // resource will not be unloaded.
void unload (Resource);
 // Unloads multiple resources at once.  This will be more efficient than
 // unloading them one at a time (the universe only needs to be scanned once),
 // and if two resources have references to eachother, they have to be unloaded
 // at the same time.  If unloading any of the resources causes an exception,
 // none of the references will be unloaded.
void unload (const std::vector<Resource>&);

 // Immediately unloads the file without scanning for references to it.  This is
 // faster, but if there are any references to data in this resource, they will
 // be left dangling.
void force_unload (Resource);
void force_unload (const std::vector<Resource>&);

 // Reloads a resource that is loaded.  Throws if the resource is not LOADED.
 // Scans all other resources for references to this one and updates them to
 // the new address.  If the reference is no longer valid, this call will throw
 // X::ReloadWouldBreak, and the resource will be restored to its old value
 // before the call to reload.  It is an error to reload while another resource
 // is being loaded.
void reload (Resource);
 // Reloads multiple resources at once.  This wlil be more efficient than
 // reloading them one at a time.  If an exception is thrown for any of the
 // resources, all of them will be restored to their old value before the call
 // to reload.
void reload (const std::vector<Resource>&);

 // Deletes the source of the resource.  If the source is a file, deletes the
 // file without confirmation.  Does not change the resource's state or value.
 // Does nothing if the source doesn't exist, but throws X::RemoveSourceFailed
 // if another error occurs (permission denied, etc.)  Calling load on the
 // resource should fail after this.
void remove_source (Resource);

 // Returns the resource currently being processed, if any.
Resource current_resource ();

 // Returns a list of all resources with state != UNLOADED.
std::vector<Resource> loaded_resources ();

///// RESOURCE NAME MANAGEMENT

 // The root directory to which file resources are relative.  You have to set
 // this before loading any resources.  Changing this changes the meaning of
 // file resource names, but the names of existing Resource objects will not
 // be updated, so they will refer to different physical files.  This cannot be
 // changed if any file resources are LOADED or LOADING.  The given filename is
 // interpreted as a directory; a trailing slash will be ignored.  Although
 // resource names are validated before being combined with this filename, this
 // filename itself will not be processed in any way.
Str file_resource_root ();
void set_file_resource_root (Str directory);
 // Set the file resource root using argv[0], so that absolute resources names
 // are relative to the directory containing the program.  This is not perfect;
 // the parent process can spoof argv[0] if it wants, but there's no portable
 // way to prevent that.
 // TODO: Scanning PATH is not implemented.  On some systems, the program may
 // fail if called from PATH.
void set_file_resource_root_from_exe (char* argv0);

 // If given name is absolute, returns it unchanged.  If it's relative, makes it
 // absolute by considering it relative to base.  If base is not given, uses
 // current_resource() as the base, and if current_resource() is not defined,
 // throws X::UnresolvedResourceName.  This also replaces // and /./ with /, and
 // collapses a/b/../c to a/c.  If the resulting name starts with /.., will
 // throw X::ResourceNameOutsideRoot.  If name (or base) has invalid characters
 // in it, throws X::InvalidResourceName.  If name is empty, returns base (or
 // current_resource()).
 // The part of base after the last / will be removed.
 //     resolve_resource_name("foo", "a/b/c") == "a/b/foo"
 //     resolve_resource_name("foo", "a/b/c/") == "a/b/c/foo"
 // TODO: Should this be internal?
String resolve_resource_name (Str name, Str base = Str());

 // Converts a resource name to a filename using file_resource_root.
String resource_filename (Str name);

///// RESOURCE HANDLERS

 // Create one of these on the top level to register a resource handler.  If a
 // resource matches this handler, its methods will be used to load, save, etc.
 // the resource.  If no handler matches a resource, it will be treated as an
 // ayu data language file.  The type managed by the header has to have a
 // AYU_DESCRIBE declaration for ayu::Type to work, but it doesn't have to
 // have any actual descriptors in the description.
struct ResourceHandler {
     // Given a resource name, returns whether this handler can handle the
     // resource.  As an example, you can check if the name ends in .png, and
     // if it does, load it as an image object.  If you're daring, you can also
     // read the file and check its magic number, though of course that won't
     // work if the file doesn't exist yet.
    virtual bool ResourceHandler_can_handle (Resource) = 0;

     // If multiple ResourceHandlers match the same name, the one with higher
     // priority will be used.  If any have equal priority, an
     // X::ResourceHandlerConflict will be thrown.
    virtual double ResourceHandler_priority () { return 0; }

     // Will be called in load() and reload() to construct the value of the
     // resource.  The resource's state will be LOAD_CONSTRUCTING.  The default
     // implementation throws X::ResourceHandlerCantLoad
    virtual void ResourceHandler_load (Resource);
     // Will be called in save() to save the resource.  First do as much
     // processing that can throw exceptions as possible, then return a
     // function that commits the result.  For example, serialize to a string,
     // then return a function that writes that string to a file.  The
     // resource's state will be SAVE_VERIFYING during this call, and
     // SAVE_COMMITTING during the callback.  The default implementation throws
     // X::ResourceHandlerCantSave
    virtual std::function<void()> ResourceHandler_save (Resource);
     // Will be called in remove_source() to delete the source.  The default
     // implementation throws X::ResourceHandlerCantRemoveSource
    virtual void ResourceHandler_remove_source (Resource);
     // Will be called in rename() after moving the value from the old resource
     // to the new one.  It's not recommended to do anything in this beside
     // update filenames or Resource objects.  The from resource's state will
     // be UNLOADED and to's state will be LOADED.  The default implementation
     // does nothing.
    virtual void ResourceHandler_after_rename (Resource from, Resource to);

     // Be default, becomes active on construction.
    ResourceHandler(bool auto_activate = true) {
        if (auto_activate) activate();
    }
    ~ResourceHandler() {
        deactivate();
    }

     // These are called in the constructor (by default) and destructor, so you
     // don't have to call them yourself.
    void activate ();
    void deactivate ();
};

///// ERRORS

namespace X {
    struct ResourceError : LogicError { };
     // Tried an an operation on a resource when its state wasn't appropriate
     // for that operation.
    struct InvalidResourceState : ResourceError {
        Str tried;
        Resource res;
        ResourceState state;
        InvalidResourceState (Str t, Resource r) :
            tried(t), res(r), state(r.state()) { }
    };
     // Tried to unload a resource, but there's still a reference somewhere
     // referencing an item inside it.
    struct UnloadWouldBreak : ResourceError {
        Path from;
        Path to;
        UnloadWouldBreak (const Path& f, const Path& t) : from(f), to(t) { }
    };
     // Tried to reload a resource, but was unable to update a reference
     // somewhere.
    struct ReloadWouldBreak : ResourceError {
        Path from;
        Path to;
        ReloadWouldBreak (const Path& f, const Path& t) : from(f), to(t) { }
    };
     // Failed to delete a resource's source file.
    struct RemoveSourceFailed : ResourceError {
        Resource res;
        int errnum; // errno
        RemoveSourceFailed (Resource r, int e) : res(r), errnum(e) { }
    };
     // Resource name contains invalid characters or something
    struct InvalidResourceName : ResourceError {
        String name;
        InvalidResourceName (String&& name) : name(std::move(name)) { }
    };
     // Resource name couldn't be resolved to a filename, such as if a
     // relative path was given but there's no current_resource().
    struct UnresolvedResourceName : ResourceError {
        String name;
        UnresolvedResourceName (String&& name) : name(std::move(name)) { }
    };
     // Resource name has too many /../s and would have left the resource root.
    struct ResourceNameOutsideRoot : ResourceError {
        String name;
        ResourceNameOutsideRoot (String&& name) : name(std::move(name)) { }
    };
     // Multiple resource handlers tried to handle the same resource with the
     // same priority.
    struct ResourceHandlerConflict : ResourceError {
        Resource res;
        double priority;
        ResourceHandlerConflict (Resource r, double p) : res(r), priority(p) { }
    };
     // Tried to load through a resource handler that's incapable of loading.
    struct ResourceHandlerCantLoad : ResourceError {
        Resource res;
        ResourceHandlerCantLoad (Resource r) : res(r) { }
    };
     // Tried to save through a resource handler that's incapable of saving.
    struct ResourceHandlerCantSave : ResourceError {
        Resource res;
        ResourceHandlerCantSave (Resource r) : res(r) { }
    };
     // Tried to remove_source through a resource handler that can't do that.
    struct ResourceHandlerCantRemoveSource : ResourceError {
        Resource res;
        ResourceHandlerCantRemoveSource (Resource r) : res(r) { }
    };
}

///// INTERNALS

namespace in {
     // TODO: make this function non-internal in case you want to scan the
     // entire universe for some reason.
    Reference universe_ref ();
    struct PushCurrentResource {
        Resource old_current;
        PushCurrentResource (Resource);
        ~PushCurrentResource ();
        PushCurrentResource (const PushCurrentResource&) = delete;
    };
}

} // namespace ayu
