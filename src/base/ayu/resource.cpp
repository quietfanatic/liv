#include "resource.h"

#include <cerrno>
#include <cstring>  // strerror
#include "compat.h"
#include "dynamic.h"
#include "describe.h"
#include "parse.h"
#include "print.h"
#include "reference.h"
#include "serialize.h"

using namespace std::literals;

///// INTERNALS

namespace hacc {
namespace in {

    struct ResourceData {
        String name;
        Dynamic value {};
        Dynamic old_value {};  // Used when reloading
        ResourceState state = UNLOADED;
    };

    struct DefaultResourceHandler : ResourceHandler {
        bool ResourceHandler_can_handle (Resource) override { return true; }
        void ResourceHandler_load (Resource res) override {
            String filename = resource_filename(res.data->name);
            item_from_tree(&res.data->value, tree_from_file(filename));
        }
        std::function<void()> ResourceHandler_save (Resource res) override {
            std::string filename = resource_filename(res.data->name);
            std::string contents = item_to_string(&res.data->value);
            return [contents{std::move(contents)}, filename{std::move(filename)}]{
                string_to_file(contents, filename);
            };
        }
        void ResourceHandler_remove_source (Resource res) override {
            auto filename = resource_filename(res.data->name);
            if (remove_utf8(filename.c_str()) != 0) {
                if (errno != ENOENT) {
                    throw X::RemoveSourceFailed(res, errno);
                }
            }
        }
        DefaultResourceHandler () : ResourceHandler(false) { }
    };

    struct Universe {
        std::unordered_map<Str, ResourceData*> resources;
        String file_resource_root;
        Resource current_resource;
        DefaultResourceHandler default_handler;
        std::vector<ResourceHandler*> handlers;
    };
    Universe& universe () {
        static Universe r;
        return r;
    }
    Reference universe_ref () {
        return &universe();
    }

    PushCurrentResource::PushCurrentResource (Resource res) :
        old_current(universe().current_resource)
    {
        universe().current_resource = res;
    }
    PushCurrentResource::~PushCurrentResource () {
        universe().current_resource = old_current;
    }

    static ResourceHandler* select_handler (Resource res) {
        auto& u = universe();
        std::vector<ResourceHandler*> hs;
        for (auto handler : u.handlers) {
            if (handler->ResourceHandler_can_handle(res)) {
                hs.push_back(handler);
            }
        }
        if (hs.empty()) return &u.default_handler;
        else {
            ResourceHandler* selected = hs[0];
            double selected_pri = hs[0]->ResourceHandler_priority();
            for (usize i = 1; i < hs.size(); i++) {
                double pri = hs[i]->ResourceHandler_priority();
                if (pri > selected_pri) {
                    selected = hs[i];
                    selected_pri = pri;
                }
                else if (pri == selected_pri) selected = null;
            }
            if (selected) return selected;
            else throw X::ResourceHandlerConflict(res, selected_pri);
        }
    }

} using namespace in;

Str show_ResourceState (ResourceState state) {
    switch (state) {
        case UNLOADED: return "UNLOADED"sv;
        case LOADED: return "LOADED"sv;
        case LOAD_CONSTRUCTING: return "LOAD_CONSTRUCTING"sv;
        case LOAD_ROLLBACK: return "LOAD_ROLLBACK"sv;
        case SAVE_VERIFYING: return "SAVE_VERIFYING"sv;
        case SAVE_COMMITTING: return "SAVE_COMMITTING"sv;
        case UNLOAD_VERIFYING: return "UNLOAD_VERIFYING"sv;
        case UNLOAD_COMMITTING: return "UNLOAD_COMMITTING"sv;
        case RELOAD_CONSTRUCTING: return "RELOAD_CONSTRUCTING"sv;
        case RELOAD_VERIFYING: return "RELOAD_VERIFYING"sv;
        case RELOAD_ROLLBACK: return "RELOAD_ROLLBACK"sv;
        case RELOAD_COMMITTING: return "RELOAD_COMMITTING"sv;
        default: HACC_INTERNAL_ERROR();
    }
}

///// RESOURCES

Resource::Resource (Str name) {
    String resolved = resolve_resource_name(name);
    auto& resources = universe().resources;
    auto iter = resources.find(resolved);
    if (iter != resources.end()) {
        data = iter->second;
    }
    else {
        data = new ResourceData{std::move(resolved)};
         // Be careful about storing the right Str (std::string_view)
        resources.emplace(data->name, data);
    }
}
Resource::Resource (Str name, Dynamic&& value) :
    Resource(name)
{
    if (data->state == UNLOADED) data->value = std::move(value);
    else throw X::InvalidResourceState("construct", *this);
}

Str Resource::name () const { return data->name; }
ResourceState Resource::state () const { return data->state; }

Dynamic& Resource::value () const {
    if (data->state == UNLOADED) {
        load(*this);
    }
    return data->value;
}
Dynamic& Resource::get_value () const {
    return data->value;
}
Dynamic& Resource::set_value () const {
    switch (data->state) {
        case UNLOADED: data->state = LOADED; return data->value;
        case LOAD_CONSTRUCTING:
        case LOADED: return data->value;
        default: throw X::InvalidResourceState("set_value"sv, data);
    }
}

Reference Resource::ref () const {
    return value();
}
Reference Resource::get_ref () const {
    if (data->state == UNLOADED) return Reference();
    else return get_value();
}

///// RESOURCE OPERATIONS

void load (Resource res) {
    std::vector<Resource> rs {res};
    load(rs);
}
void load (const std::vector<Resource>& reses) {
    std::vector<Resource> rs;
    for (auto res : reses)
    switch (res.data->state) {
        case UNLOADED: rs.push_back(res); break;
        case LOADED:
        case LOAD_CONSTRUCTING: continue;
        default: throw X::InvalidResourceState("load"sv, res);
    }
    try {
        for (auto res : rs) {
            res.data->state = LOAD_CONSTRUCTING;
        }
        for (auto res : rs) {
            PushCurrentResource p (res);
            auto handler = select_handler(res);
            handler->ResourceHandler_load(res);
        }
        for (auto res : rs) {
            res.data->state = LOADED;
        }
    }
    catch (...) {
        for (auto res : rs) {
            res.data->state = LOAD_ROLLBACK;
        }
        for (auto res : rs) {
            try {
                res.data->value = Dynamic();
            }
            catch (std::exception& e) {
                unrecoverable_exception(e, "while rolling back load"sv);
            }
            res.data->state = UNLOADED;
        }
        throw;
    }
}

void rename (Resource old_res, Resource new_res) {
    if (old_res.data->state != LOADED) {
        throw X::InvalidResourceState("rename from"sv, old_res);
    }
    if (new_res.data->state != UNLOADED) {
        throw X::InvalidResourceState("rename to"sv, new_res);
    }
    new_res.data->value = std::move(old_res.data->value);
    new_res.data->state = LOADED;
    old_res.data->state = UNLOADED;
    PushCurrentResource p (new_res);
    auto handler = select_handler(new_res);
    handler->ResourceHandler_after_rename(old_res, new_res);
}

void save (Resource res) {
    std::vector<Resource> reses {res};
    save(reses);
}
void save (const std::vector<Resource>& reses) {
    for (auto res : reses) {
        if (res.data->state != LOADED) {
            throw X::InvalidResourceState("save", res);
        }
    }
    try {
        for (auto res : reses) {
            res.data->state = SAVE_VERIFYING;
        }
         // Serialize all before writing to disk
        std::vector<std::function<void()>> committers (reses.size());
        for (usize i = 0; i < reses.size(); i++) {
            PushCurrentResource p (reses[i]);
            auto handler = select_handler(reses[i]);
            committers[i] = handler->ResourceHandler_save(reses[i]);
        }
        for (auto res : reses) {
            res.data->state = SAVE_COMMITTING;
        }
        for (usize i = 0; i < reses.size(); i++) {
            committers[i]();
        }
        for (auto res : reses) {
            res.data->state = LOADED;
        }
    }
    catch (...) {
        for (auto res : reses) {
            res.data->state = LOADED;
        }
        throw;
    }
}

void unload (Resource res) {
    std::vector<Resource> reses {res};
    unload(reses);
}
void unload (const std::vector<Resource>& reses) {
    static Type ref_type = Type::CppType<Reference>();
    std::vector<Resource> rs;
    for (auto res : reses)
    switch (res.data->state) {
        case UNLOADED: continue;
        case LOADED: rs.push_back(res); break;
        default: throw X::InvalidResourceState("unload"sv, res);
    }
     // Verify step
    try {
        for (auto res : rs) {
            res.data->state = UNLOAD_VERIFYING;
        }
        std::vector<Resource> others;
        for (auto& [name, other] : universe().resources) {
            switch (other->state) {
                case UNLOADED: continue;
                case UNLOAD_VERIFYING: continue;
                case LOADED: others.emplace_back(other); break;
                default: throw X::InvalidResourceState("scan for unload"sv, other);
            }
        }
         // If we're unloading everything, no need to do any scanning.
        if (!others.empty()) {
             // First build set of references to things being unloaded
            std::unordered_map<Reference, Path> ref_set;
            for (auto res : rs) {
                recursive_scan(
                    res.data->value, Path(Path(), res.data->name),
                    [&](const Reference& ref, Path path) {
                        ref_set.emplace(ref, path);
                    }
                );
            }
             // Then check if any other resources contain references in that set
            for (auto other : others) {
                recursive_scan(other.data->value, Path(Path(), other.data->name),
                    [&](Reference ref_ref, Path path) {
                        if (ref_ref.type() != ref_type) return;
                        Reference ref = ref_ref.get_as<Reference>();
                        auto iter = ref_set.find(ref);
                        if (iter != ref_set.end()) {
                            throw X::UnloadWouldBreak(path, iter->second);
                        }
                    }
                );
            }
        }
         // If we got here, no references will be broken by the unload
    }
    catch (...) {
        for (auto res : rs) {
            res.data->state = LOADED;
        }
        throw;
    }
     // Destruct step
    for (auto res : rs) {
        res.data->state = UNLOAD_COMMITTING;
    }
    for (auto res : rs) {
        try {
            res.data->value = Dynamic();
        }
        catch (std::exception& e) {
            unrecoverable_exception(e, "while running destructor during unload"sv);
        }
        res.data->state = UNLOADED;
    }
}

void force_unload (Resource res) {
    std::vector<Resource> reses {res};
    force_unload(reses);
}
void force_unload (const std::vector<Resource>& reses) {
    std::vector<Resource> rs;
    for (auto res : reses)
    switch (res.data->state) {
        case UNLOADED: continue;
        case LOADED: rs.push_back(res); break;
        default: throw X::InvalidResourceState("force_unload"sv, res);
    }
     // Skip straight to destruct step
    for (auto res : rs) {
        res.data->state = UNLOAD_COMMITTING;
    }
    for (auto res : rs) {
        try {
            res.data->value = Dynamic();
        }
        catch (std::exception& e) {
            unrecoverable_exception(e, "while running destructor during force_unload"sv);
        }
        res.data->state = UNLOADED;
    }
}

void reload (Resource res) {
    std::vector<Resource> reses {res};
    reload(reses);
}
void reload (const std::vector<Resource>& reses) {
    static Type ref_type = Type::CppType<Reference>();
    for (auto res : reses)
    if (res.data->state != LOADED) {
        throw X::InvalidResourceState("reload"sv, res);
    }
     // Preparation (this won't throw)
    for (auto res : reses) {
        res.data->state = RELOAD_CONSTRUCTING;
        res.data->old_value = std::move(res.data->value);
    }
    std::unordered_map<Reference, Reference> updates;
    try {
         // Construct step
        for (auto res : reses) {
            PushCurrentResource p (res);
            auto handler = select_handler(res);
            handler->ResourceHandler_load(res);
        }
        for (auto res : reses) {
            res.data->state = RELOAD_VERIFYING;
        }
         // Verify step
        std::vector<Resource> others;
        for (auto& [name, other] : universe().resources) {
            switch (other->state) {
                case UNLOADED: continue;
                case RELOAD_VERIFYING: continue;
                case LOADED: others.emplace_back(other); break;
                default: throw X::InvalidResourceState("scan for reload"sv, other);
            }
        }
         // If we're reloading everything, no need to do any scanning.
        if (!others.empty()) {
             // First build mapping of old refs to paths
            std::unordered_map<Reference, Path> old_refs;
            for (auto res : reses) {
                recursive_scan(
                    res.data->old_value, Path(Path(), res.data->name),
                    [&](const Reference& ref, Path path) {
                        old_refs.emplace(ref, path);
                    }
                );
            }
             // Then build set of ref-refs to update.
            for (auto other : others) {
                recursive_scan(other.data->value, Path(Path(), other.data->name),
                    [&](Reference ref_ref, Path path) {
                        if (ref_ref.type() != ref_type) return;
                        Reference ref = ref_ref.get_as<Reference>();
                        auto iter = old_refs.find(ref);
                        if (iter == old_refs.end()) return;
                        try {
                             // reference_from_path will use new resource value
                            Reference new_ref = reference_from_path(iter->second);
                            updates.emplace(ref_ref, new_ref);
                        }
                        catch (X::Error&) {
                             // I think it's okay to throw away the error info
                            throw X::ReloadWouldBreak(path, iter->second);
                        }
                    }
                );
            }
        }
    }
    catch (...) {
        for (auto res : reses) {
            res.data->state = RELOAD_ROLLBACK;
        }
        for (auto res : reses) {
            try {
                res.data->value = Dynamic();
            }
            catch (std::exception& e) {
                unrecoverable_exception(e, "while rolling back reload"sv);
            }
            res.data->value = std::move(res.data->old_value);
        }
        for (auto res : reses) {
            res.data->state = LOADED;
        }
    }
     // Commit step
    for (auto& [ref_ref, new_ref] : updates) {
        try {
            if (auto a = ref_ref.address()) {
                reinterpret_cast<Reference&>(*a) = new_ref;
            }
            else ref_ref.write([&](Mu& v){
                reinterpret_cast<Reference&>(v) = new_ref;
            });
        }
        catch (std::exception& e) {
            unrecoverable_exception(e, "while updating references for reload"sv);
        }
    }
     // Destruct step
    for (auto res : reses) {
        res.data->state = RELOAD_COMMITTING;
    }
    for (auto res : reses) {
        try {
            res.data->value = Dynamic();
        }
        catch (std::exception& e) {
            unrecoverable_exception(e, "while destructing old values for reload"sv);
        }
    }
    for (auto res : reses) {
        res.data->state = LOADED;
    }
}

void remove_source (Resource res) {
    PushCurrentResource p (res);
    auto handler = select_handler(res);
    handler->ResourceHandler_remove_source(res);
}

Resource current_resource () {
    return universe().current_resource;
}

std::vector<Resource> loaded_resources () {
    std::vector<Resource> r;
    for (auto& [name, rd] : universe().resources)
    if (rd->state != UNLOADED) {
        r.push_back(rd);
    }
    return r;
}

///// NAME MANAGEMENT

Str file_resource_root () {
    return universe().file_resource_root;
}
void set_file_resource_root (Str root) {
    universe().file_resource_root = String(root);
}
void set_file_resource_root_from_exe (char* argv0) {
    Str exe = argv0;
    usize pos = exe.rfind('/');
    usize pos2 = exe.rfind('\\');
    if (pos != Str::npos && pos2 != Str::npos) {
        throw X::GenericError("argv[0] contains both / and \\, I am confused.");
    }
    if (pos == Str::npos) pos = pos2;
    if (pos == Str::npos) {
        throw X::GenericError("Can't find the executable location based on argv[0] (scanning PATH is NYI).  argv[0]: "s + exe);
    }
    set_file_resource_root(exe.substr(0, pos));
}

static void split_name_to (std::vector<Str>& segments, Str name) {
    usize start = 0;
    usize i;
    for (i = 0; i < name.size(); i++) {
        if (name[i] == '/') {
            if (i != start) {
                segments.push_back(name.substr(start, i - start));
            }
            start = i+1;
        }
    }
     // If name ends with /, will push an empty segment
    segments.push_back(name.substr(start, i - start));
}

static String join_name (const std::vector<Str>& segments) {
    String r;
    for (auto seg : segments) {
        r += "/"s + seg;
    }
    return r;
}

String resolve_resource_name (Str name, Str base) {
     // Validate
    for (auto c : name)
    if (c == ':' || c == '?' || c == '#') {
        throw X::InvalidResourceName(String(name));
    }
     // Resolve base first
    if (!base.empty()) {
        base = resolve_resource_name(base);
    }
    else {
        auto cur = current_resource();
        if (cur) base = cur.name();
    }
     // Empty name refers to base (or current_resource)
    if (name.empty()) {
        if (base.empty()) throw X::UnresolvedResourceName(String(name));
        else return String(base);
    }
     // Concatenate
    std::vector<Str> segments;
    if (name[0] != '/') {
        split_name_to(segments, base);
        segments.pop_back();  // Make relative to base's directory, not base itself
    }
    split_name_to(segments, name);
     // Normalize
    std::vector<Str> normalized;
    bool outside_root = false;
    for (auto seg : segments) {
        if (seg == "."sv) continue;
        else if (seg == ".."sv) {
            if (normalized.empty()) outside_root = true;
            else normalized.pop_back();
        }
        else normalized.push_back(seg);
    }
    String r = join_name(normalized);
    if (outside_root) throw X::ResourceNameOutsideRoot(std::move(r));
    return r;
}

String resource_filename (Str name) {
    auto& root = universe().file_resource_root;
    if (root.empty()) {
        throw X::GenericError("Cannot get resource filenames until set_file_resource_root is called");
    }
    return root + resolve_resource_name(name);
}

///// RESOURCE HANDLERS

void ResourceHandler::ResourceHandler_load (Resource res) {
    throw X::ResourceHandlerCantLoad(res);
}
std::function<void()> ResourceHandler::ResourceHandler_save (Resource res) {
    throw X::ResourceHandlerCantSave(res);
}
void ResourceHandler::ResourceHandler_remove_source (Resource res) {
    throw X::ResourceHandlerCantRemoveSource(res);
}
void ResourceHandler::ResourceHandler_after_rename (Resource, Resource) { }

void ResourceHandler::activate () {
    auto& handlers = universe().handlers;
    for (auto handler : handlers) {
        if (handler == this) return;
    }
    handlers.push_back(this);
}
void ResourceHandler::deactivate () {
    auto& handlers = universe().handlers;
    for (auto it = handlers.begin(); it != handlers.end(); it++) {
        if (*it == this) {
            handlers.erase(it);
            return;
        }
    }
}

} using namespace hacc;

///// HACCABILITY

HACCABLE(hacc::in::Universe,
    keys(value_func<std::vector<String>>([](const Universe& v){
        std::vector<String> ks;
        for (auto& p : v.resources) {
            if (p.second && p.second->state != UNLOADED) {
                ks.emplace_back(p.first);
            }
        }
        return ks;
    })),
    attr_func([](Universe&, Str key){
         // Resources always have to be Dynamic, so go ahead and reference the
         //  Dynamic's value instead of the Dynamic object itself (saves a 1
         //  in paths)
        return Reference(Resource(key).value());
    })
)

HACCABLE(hacc::Resource,
    delegate(mixed_funcs<String>(
        [](const Resource& v){
            return v.data->name;
        },
        [](Resource& v, const String& m){
            v = Resource(m);
        }
    ))
)

HACCABLE(hacc::X::InvalidResourceState,
    elems(
        elem(&X::InvalidResourceState::tried),
        elem(&X::InvalidResourceState::state),
        elem(&X::InvalidResourceState::res)
    )
)
HACCABLE(hacc::X::UnloadWouldBreak,
    elems(
        elem(&X::UnloadWouldBreak::from),
        elem(&X::UnloadWouldBreak::to)
    )
)
HACCABLE(hacc::X::ReloadWouldBreak,
    elems(
        elem(&X::ReloadWouldBreak::from),
        elem(&X::ReloadWouldBreak::to)
    )
)
HACCABLE(hacc::X::RemoveSourceFailed,
    elems(
        elem(&X::RemoveSourceFailed::res),
        elem(value_func<String>(
            [](const X::RemoveSourceFailed& v){
                return String(std::strerror(v.errnum));
            }
        ))
    )
)
HACCABLE(hacc::X::InvalidResourceName,
    elems(elem(&X::InvalidResourceName::name))
)
HACCABLE(hacc::X::UnresolvedResourceName,
    elems(elem(&X::UnresolvedResourceName::name))
)
HACCABLE(hacc::X::ResourceNameOutsideRoot,
    elems(elem(&X::ResourceNameOutsideRoot::name))
)
HACCABLE(hacc::X::ResourceHandlerConflict,
    elems(
        elem(&X::ResourceHandlerConflict::res),
        elem(&X::ResourceHandlerConflict::priority)
    )
)
HACCABLE(hacc::X::ResourceHandlerCantLoad,
    elems(elem(&X::ResourceHandlerCantLoad::res))
)
HACCABLE(hacc::X::ResourceHandlerCantSave,
    elems(elem(&X::ResourceHandlerCantSave::res))
)
HACCABLE(hacc::X::ResourceHandlerCantRemoveSource,
    elems(elem(&X::ResourceHandlerCantRemoveSource::res))
)

///// TESTS

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"
#include "document.h"
#include "describe-standard.h"

static tap::TestSet tests ("base/ayu/resource", []{
    using namespace tap;
     // Note: We're relying on the caller to set the file resource root!
    Resource input ("/base/ayu/test/testfile.ayu");
    Resource input2 ("/base/ayu/test/othertest.ayu");
    Resource rec1 ("/base/ayu/test/rec1.ayu");
    Resource rec2 ("/base/ayu/test/rec2.ayu");
    Resource badinput ("/base/ayu/test/badref.ayu");
    Resource output ("/base/ayu/test/test-output.ayu");
    Resource unicode ("/base/ayu/test/ユニコード.ayu");
    Resource unicode2 ("/base/ayu/test/ユニコード2.ayu");

    is(input.state(), UNLOADED, "Resources start out unloaded");
    doesnt_throw([&]{ load(input); }, "load");
    is(input.state(), LOADED, "Resource state is LOADED after loading");
    ok(input.value().has_value(), "Resource has value after loading");

    throws<X::InvalidResourceState>([&]{
        Resource(input.name(), Dynamic(3));
    }, "Creating resource throws on duplicate");

    doesnt_throw([&]{ unload(input); }, "unload");
    is(input.state(), UNLOADED, "Resource state is UNLOADED after unloading");
    ok(!input.data->value.has_value(), "Resource has no value after unloading");

    hacc::Document* doc = null;
    doesnt_throw([&]{
        doc = &input.value().as<hacc::Document>();
    }, "Getting typed value from a resource");
    is(input.state(), LOADED, "Resource::value() automatically loads resource");
    is(input["foo"][1].get_as<int32>(), 4, "Value was generated properly (0)");
    is(input["bar"][1].get_as<std::string>(), "qux", "Value was generated properly (1)");

    throws<X::InvalidResourceState>([&]{ save(output); }, "save throws on unloaded resource");

    doc->delete_named("foo");
    doc->new_named<int32>("asdf", 51);

    doesnt_throw([&]{ rename(input, output); }, "rename");
    is(input.state(), UNLOADED, "Old resource is UNLOADED after renaming");
    is(output.state(), LOADED, "New resource is LOADED after renaming");
    is(&output.value().as<hacc::Document>(), doc, "Rename moves value without reconstructing it");

    doesnt_throw([&]{ save(output); }, "save");
    is(tree_from_file(resource_filename(output.name())), tree_from_string(
        "[hacc::Document {bar:[std::string qux] asdf:[int32 51] _next_id:0}]"
    ), "Resource was saved with correct contents");
    doesnt_throw([&]{ remove_source(output); }, "remove_source");
    throws<X::OpenFailed>([&]{
        tree_from_file(resource_filename(output.name()));
    }, "Can't open file after calling remove_source");
    doesnt_throw([&]{ remove_source(output); }, "Can call remove_source twice");
    Path path;
    doesnt_throw([&]{
        item_from_string(&path, "[\"" + input.name() + "\" bar 1]");
    }, "Can read path from tree");
    Reference ref;
    doesnt_throw([&]{
        ref = reference_from_path(path);
    }, "reference_from_path");
    doesnt_throw([&]{
        is(ref.get_as<std::string>(), "qux", "reference_from_path got correct item");
    });
    doc = &output.value().as<hacc::Document>();
    ref = output["asdf"][1].address_as<int32>();
    doesnt_throw([&]{
        path = reference_to_path(ref);
    });
    is(item_to_tree(&path), tree_from_string("[\"" + output.name() + "\" asdf 1]"), "reference_to_path works");
    doc->new_<Reference>(output["bar"][1]);
    doesnt_throw([&]{ save(output); }, "save with reference");
    doc->new_<int32*>(output["asdf"][1]);
    doesnt_throw([&]{ save(output); }, "save with pointer");
    is(tree_from_file(resource_filename(output.name())), tree_from_string(
        "[hacc::Document {bar:[std::string qux] asdf:[int32 51] _0:[hacc::Reference [\"" + output.name() + "\" bar 1]] _1:[int32* [\"" + output.name() + "\" asdf 1]] _next_id:2}]"
    ), "File was saved with correct reference as path");
    throws<X::OpenFailed>([&]{
        load(badinput);
    }, "Can't load file with incorrect reference in it");

    doesnt_throw([&]{
        unload(input);
        load(input2);
    }, "Can load second file referencing first");
    is(Resource(input).state(), LOADED, "Loading second file referencing first file loads first file");
    std::string* bar;
    doesnt_throw([&]{
        bar = input["bar"][1];
    }, "can use [] syntax on resources and references");
    is(
        input2["ext_pointer"][1].get_as<std::string*>(),
        bar,
        "Loading a pointer worked!"
    );

    int asdf = 0;
    doesnt_throw([&]{
        asdf = *unicode["ptr"][1].get_as<int*>();
    }, "Can load and reference files with unicode in their name");
    is(asdf, 4444);

    is(
        unicode2["self_pointer"][1].get_as<std::string*>(),
        unicode2["val"][1].address_as<std::string>(),
        "Loading pointer with \"\" for own file worked."
    );
    throws<X::UnloadWouldBreak>([&]{
        unload(input);
    }, "Can't unload resource when there are references to it");
    doesnt_throw([&]{
        unload(input2);
        unload(input);
    }, "Can unload if we unload the referring resource first");
    doesnt_throw([&]{
        load(rec1);
    }, "Can load resources with reference cycle");
    throws<X::UnloadWouldBreak>([&]{
        unload(rec1);
    }, "Can't unload part of a reference cycle 1");
    throws<X::UnloadWouldBreak>([&]{
        unload(rec2);
    }, "Can't unload part of a reference cycle 2");
    doesnt_throw([&]{
        unload({rec1, rec2});
    }, "Can unload reference cycle by unload both resources at once");
    load(rec1);
    int* old_p = rec1["ref"][1].get_as<int*>();
    doesnt_throw([&]{
        reload(rec2);
    }, "Can reload file with references to it");
    isnt(rec1["ref"][1].get_as<int*>(), old_p, "Reference to reloaded file was updated");

    done_testing();
});
#endif
