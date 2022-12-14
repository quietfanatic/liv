#include "../resource.h"

#include <cerrno>
#include <cstring>  // strerror
#include "../../uni/utf.h"
#include "../dynamic.h"
#include "../describe.h"
#include "../describe-standard.h"
#include "../parse.h"
#include "../print.h"
#include "../reference.h"
#include "../resource-scheme.h"
#include "../scan.h"
#include "../serialize.h"
#include "universe-private.h"

///// INTERNALS

namespace ayu {
namespace in {

    inline void verify_tree_for_scheme (
        Resource res,
        const ResourceScheme* scheme,
        const Tree& tree
    ) {
        if (tree.form == NULLFORM) {
            throw X<EmptyResourceValue>(std::string(res.name().spec()));
        }
        const Array& a = static_cast<const Array&>(tree);
        if (a.size() == 2) {
            Type type = Type(Str(a[0]));
            if (!scheme->accepts_type(type)) {
                throw X<UnacceptableResourceType>{
                    std::string(res.name().spec()), type
                };
            }
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
        default: never();
    }
}

///// RESOURCES

 // These are separate because it's not known at call time whether we will need
 // to copy or not.  We won't know until just about the end of the constructor.
Resource::Resource (const IRI& name) {
    if (name.has_fragment()) {
        new (this) Resource(name.iri_without_fragment());
        return;
    }
    if (!name) {
        throw X<InvalidResourceName>(std::string(name.possibly_invalid_spec()));
    }
    auto scheme = universe().require_scheme(name);
    if (!scheme->accepts_iri(name)) {
        throw X<UnacceptableResourceName>(std::string(name.spec()));
    }
    auto& resources = universe().resources;
    auto iter = resources.find(name.spec());
    if (iter != resources.end()) {
        data = &*iter->second;
    }
    else {
        auto ptr = std::make_unique<ResourceData>(name);
        data = &*ptr;
         // Be careful about storing the right Str (std::string_view)
        resources.emplace(data->name.spec(), std::move(ptr));
    }
}
Resource::Resource (IRI&& name) {
    if (name.has_fragment()) {
        new (this) Resource(name.iri_without_fragment());
    }
    if (!name) {
        throw X<InvalidResourceName>(std::string(name.possibly_invalid_spec()));
    }
    auto scheme = universe().require_scheme(name);
    if (!scheme->accepts_iri(name)) {
        throw X<UnacceptableResourceName>(std::string(name.spec()));
    }
    auto& resources = universe().resources;
    auto iter = resources.find(name.spec());
    if (iter != resources.end()) {
        data = &*iter->second;
    }
    else {
        auto ptr = std::make_unique<ResourceData>(name);
        data = &*ptr;
         // Be careful about storing the right Str (std::string_view)
        resources.emplace(data->name.spec(), std::move(ptr));
    }
}
Resource::Resource (Str ref) {
    if (auto res = current_resource()) {
        if (ref == "#"sv) new (this) Resource(res.data->name);
        else new (this) Resource (IRI(ref, res.data->name));
    }
    else new (this) Resource(IRI(ref));
}

Resource::Resource (IRI name, Dynamic&& value) :
    Resource(std::move(name))
{
    if (!value.has_value()) {
        throw X<EmptyResourceValue>(std::string(name.spec()));
    }
    if (data->state == UNLOADED) set_value(std::move(value));
    else throw X<InvalidResourceState>("construct"sv, *this, data->state);
}

const IRI& Resource::name () const { return data->name; }
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
void Resource::set_value (Dynamic&& value) const {
    if (!value.has_value()) {
        throw X<EmptyResourceValue>(std::string(data->name.spec()));
    }
    if (data->name) {
        auto scheme = universe().require_scheme(data->name);
        if (!scheme->accepts_type(value.type)) {
            throw X<UnacceptableResourceType>{
                std::string(data->name.spec()), value.type
            };
        }
    }
    switch (data->state) {
        case UNLOADED:
            data->state = LOADED;
            break;
        case LOAD_CONSTRUCTING:
        case LOADED:
            break;
        default: throw X<InvalidResourceState>("set_value"sv, data, data->state);
    }
    data->value = std::move(value);
}

Reference Resource::ref () const {
    return value().ptr();
}
Reference Resource::get_ref () const {
    if (data->state == UNLOADED) return Reference();
    else return get_value().ptr();
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
        default: throw X<InvalidResourceState>("load"sv, res, res.data->state);
    }
    try {
        for (auto res : rs) {
            res.data->state = LOAD_CONSTRUCTING;
        }
        for (auto res : rs) {
            auto scheme = universe().require_scheme(res.data->name);
            std::string filename = scheme->get_file(res.data->name);
            Tree tree = tree_from_file(filename);
            verify_tree_for_scheme(res, scheme, tree);
            item_from_tree(
                &res.data->value, tree, Location(res), DELAY_SWIZZLE
            );
        }
        for (auto res : rs) {
            res.data->state = LOADED;
        }
    }
    catch (...) {
         // TODO: When recursing load(), rollback innerly loading resources if
         // an outerly loading resource throws.
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
        throw X<InvalidResourceState>("rename from"sv, old_res, old_res.data->state);
    }
    if (new_res.data->state != UNLOADED) {
        throw X<InvalidResourceState>("rename to"sv, new_res, old_res.data->state);
    }
    new_res.data->value = std::move(old_res.data->value);
    new_res.data->state = LOADED;
    old_res.data->state = UNLOADED;
}

void save (Resource res) {
    std::vector<Resource> reses {res};
    save(reses);
}
void save (const std::vector<Resource>& reses) {
    for (auto res : reses) {
        if (res.data->state != LOADED) {
            throw X<InvalidResourceState>("save"sv, res, res.data->state);
        }
    }
    try {
        for (auto res : reses) {
            res.data->state = SAVE_VERIFYING;
        }
         // Serialize all before writing to disk
        std::vector<std::function<void()>> committers (reses.size());
        {
            KeepLocationCache klc;
            for (usize i = 0; i < reses.size(); i++) {
                Resource res = reses[i];
                if (!res.data->value.has_value()) {
                    throw X<EmptyResourceValue>(std::string(res.data->name.spec()));
                }
                auto scheme = universe().require_scheme(res.data->name);
                if (!scheme->accepts_type(res.data->value.type)) {
                    throw X<UnacceptableResourceType>{
                        std::string(res.data->name.spec()),
                        res.data->value.type
                    };
                }
                std::string filename = scheme->get_file(res.data->name);
                auto contents = tree_to_string(
                    item_to_tree(&res.data->value, Location(res))
                );
                committers[i] = [
                    contents{std::move(contents)},
                    filename{std::move(filename)}
                ]{
                    string_to_file(contents, filename);
                };
            }
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
    std::vector<Resource> rs;
    for (auto res : reses)
    switch (res.data->state) {
        case UNLOADED: continue;
        case LOADED: rs.push_back(res); break;
        default: throw X<InvalidResourceState>("unload"sv, res, res.data->state);
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
                case LOADED: others.emplace_back(&*other); break;
                default: throw X<InvalidResourceState>("scan for unload"sv, &*other, other->state);
            }
        }
         // If we're unloading everything, no need to do any scanning.
        if (!others.empty()) {
             // First build set of references to things being unloaded
            std::unordered_map<Reference, Location> ref_set;
            for (auto res : rs) {
                scan_resource_references(
                    res,
                    [&](const Reference& ref, LocationRef loc) {
                        ref_set.emplace(ref, loc);
                        return false;
                    }
                );
            }
             // Then check if any other resources contain references in that set
            for (auto other : others) {
                scan_resource_references(
                    other,
                    [&](Reference ref_ref, LocationRef loc) {
                         // TODO: Check for Pointer as well
                        if (ref_ref.type() != Type::CppType<Reference>()) return false;
                        Reference ref = ref_ref.get_as<Reference>();
                        auto iter = ref_set.find(ref);
                        if (iter != ref_set.end()) {
                            throw X<UnloadWouldBreak>(loc, iter->second);
                        }
                        return false;
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
    try {
        for (auto res : rs) {
            res.data->value = Dynamic();
            res.data->state = UNLOADED;
        }
    }
    catch (std::exception& e) {
        unrecoverable_exception(e, "while running destructor during unload"sv);
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
        default: throw X<InvalidResourceState>("force_unload"sv, res, res.data->state);
    }
     // Skip straight to destruct step
    for (auto res : rs) {
        res.data->state = UNLOAD_COMMITTING;
    }
    try {
        for (auto res : rs) {
            res.data->value = Dynamic();
            res.data->state = UNLOADED;
        }
    }
    catch (std::exception& e) {
        unrecoverable_exception(e, "while running destructor during force_unload"sv);
    }
}

void reload (Resource res) {
    std::vector<Resource> reses {res};
    reload(reses);
}
void reload (const std::vector<Resource>& reses) {
    for (auto res : reses)
    if (res.data->state != LOADED) {
        throw X<InvalidResourceState>("reload"sv, res, res.data->state);
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
            auto scheme = universe().require_scheme(res.data->name);
            std::string filename = scheme->get_file(res.data->name);
            Tree tree = tree_from_file(filename);
            verify_tree_for_scheme(res, scheme, tree);
             // Do not DELAY_SWIZZLE for reload.  TODO: Forbid reload while a
             // serialization operation is ongoing.
            item_from_tree(&res.data->value, tree, Location(res));
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
                case LOADED: others.emplace_back(&*other); break;
                default: throw X<InvalidResourceState>("scan for reload"sv, &*other, other->state);
            }
        }
         // If we're reloading everything, no need to do any scanning.
        if (!others.empty()) {
             // First build mapping of old refs to locationss
            std::unordered_map<Reference, Location> old_refs;
            for (auto res : reses) {
                scan_references(
                    res.data->old_value.ptr(), Location(res),
                    [&](const Reference& ref, LocationRef loc) {
                        old_refs.emplace(ref, loc);
                        return false;
                    }
                );
            }
             // Then build set of ref-refs to update.
            for (auto other : others) {
                scan_resource_references(
                    other,
                    [&](Reference ref_ref, LocationRef loc) {
                         // TODO: scan Pointers as well
                        if (ref_ref.type() != Type::CppType<Reference>()) return false;
                        Reference ref = ref_ref.get_as<Reference>();
                        auto iter = old_refs.find(ref);
                        if (iter == old_refs.end()) return false;
                        try {
                             // reference_from_location will use new resource value
                            Reference new_ref = reference_from_location(iter->second);
                            updates.emplace(ref_ref, new_ref);
                        }
                        catch (Error&) {
                             // It's probably okay to throw away the error info
                            throw X<ReloadWouldBreak>(loc, iter->second);
                        }
                        return false;
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
        throw;
    }
     // Commit step
    try {
        for (auto& [ref_ref, new_ref] : updates) {
            if (auto a = ref_ref.address()) {
                reinterpret_cast<Reference&>(*a) = new_ref;
            }
            else ref_ref.write([&](Mu& v){
                reinterpret_cast<Reference&>(v) = new_ref;
            });
        }
    }
    catch (std::exception& e) {
        unrecoverable_exception(e, "while updating references for reload"sv);
    }
     // Destruct step
    for (auto res : reses) {
        res.data->state = RELOAD_COMMITTING;
    }
    try {
        for (auto res : reses) {
            res.data->value = Dynamic();
        }
    }
    catch (std::exception& e) {
        unrecoverable_exception(e, "while destructing old values for reload"sv);
    }
    for (auto res : reses) {
        res.data->state = LOADED;
    }
}

std::string resource_filename (Resource res) {
    auto scheme = universe().require_scheme(res.data->name);
    return scheme->get_file(res.data->name);
}

void remove_source (Resource res) {
    auto scheme = universe().require_scheme(res.data->name);
    std::string filename = scheme->get_file(res.data->name);
    remove_utf8(filename.c_str());
}

bool source_exists (Resource res) {
    auto scheme = universe().require_scheme(res.data->name);
    std::string filename = scheme->get_file(res.data->name);
    if (std::FILE* f = fopen_utf8(filename.c_str())) {
        fclose(f);
        return true;
    }
    else return false;
}

Resource current_resource () {
    if (auto res = current_location().root_resource()) {
        return *res;
    }
    else return Resource();
}

std::vector<Resource> loaded_resources () {
    std::vector<Resource> r;
    for (auto& [name, rd] : universe().resources)
    if (rd->state != UNLOADED) {
        r.push_back(&*rd);
    }
    return r;
}

} using namespace ayu;

///// DESCRIPTIONS

AYU_DESCRIBE(ayu::Resource,
    delegate(const_ref_funcs<IRI>(
        [](const Resource& v) -> const IRI& {
            return v.data->name;
        },
        [](Resource& v, const IRI& m){
            v = Resource(m);
        }
    ))
)
AYU_DESCRIBE(ayu::ResourceState,
    values(
        value("UNLOADED", UNLOADED),
        value("LOADED", LOADED),
        value("LOAD_CONSTRUCTING", LOAD_CONSTRUCTING),
        value("LOAD_ROLLBACK", LOAD_ROLLBACK),
        value("SAVE_VERIFYING", SAVE_VERIFYING),
        value("SAVE_COMMITTING", SAVE_COMMITTING),
        value("UNLOAD_VERIFYING", UNLOAD_VERIFYING),
        value("UNLOAD_COMMITTING", UNLOAD_COMMITTING),
        value("RELOAD_CONSTRUCTING", RELOAD_CONSTRUCTING),
        value("RELOAD_VERIFYING", RELOAD_VERIFYING),
        value("RELOAD_ROLLBACK", RELOAD_ROLLBACK),
        value("RELOAD_COMMITTING", RELOAD_COMMITTING)
    )
)

AYU_DESCRIBE(ayu::ResourceError,
    delegate(base<Error>())
)
AYU_DESCRIBE(ayu::InvalidResourceState,
    elems(
        elem(base<ResourceError>(), inherit),
        elem(&InvalidResourceState::tried),
        elem(&InvalidResourceState::state),
        elem(&InvalidResourceState::res)
    )
)
AYU_DESCRIBE(ayu::EmptyResourceValue,
    elems(
        elem(base<ResourceError>(), inherit),
        elem(&EmptyResourceValue::name)
    )
)
AYU_DESCRIBE(ayu::UnloadWouldBreak,
    elems(
        elem(base<ResourceError>(), inherit),
        elem(&UnloadWouldBreak::from),
        elem(&UnloadWouldBreak::to)
    )
)
AYU_DESCRIBE(ayu::ReloadWouldBreak,
    elems(
        elem(base<ResourceError>(), inherit),
        elem(&ReloadWouldBreak::from),
        elem(&ReloadWouldBreak::to)
    )
)
AYU_DESCRIBE(ayu::RemoveSourceFailed,
    elems(
        elem(base<ResourceError>(), inherit),
        elem(&RemoveSourceFailed::res),
        elem(value_func<std::string>(
            [](const RemoveSourceFailed& v){
                return std::string(std::strerror(v.errnum));
            }
        ))
    )
)

///// TESTS

#ifndef TAP_DISABLE_TESTS
#include "test-environment-private.h"

AYU_DESCRIBE_INSTANTIATE(std::unordered_set<int32*>)

static tap::TestSet tests ("base/ayu/resource", []{
    using namespace tap;

    test::TestEnvironment env;

    Resource input ("ayu-test:/testfile.ayu");
    Resource input2 ("ayu-test:/othertest.ayu");
    Resource rec1 ("ayu-test:/rec1.ayu");
    Resource rec2 ("ayu-test:/rec2.ayu");
    Resource badinput ("ayu-test:/badref.ayu");
    Resource output ("ayu-test:/test-output.ayu");
    Resource unicode ("ayu-test:/???????????????.ayu");
    Resource unicode2 ("ayu-test:/???????????????2.ayu");

    is(input.state(), UNLOADED, "Resources start out unloaded");
    doesnt_throw([&]{ load(input); }, "load");
    is(input.state(), LOADED, "Resource state is LOADED after loading");
    ok(input.value().has_value(), "Resource has value after loading");

    throws<InvalidResourceState>([&]{
        Resource(input.name(), Dynamic(3));
    }, "Creating resource throws on duplicate");

    doesnt_throw([&]{ unload(input); }, "unload");
    is(input.state(), UNLOADED, "Resource state is UNLOADED after unloading");
    ok(!input.data->value.has_value(), "Resource has no value after unloading");

    ayu::Document* doc = null;
    doesnt_throw([&]{
        doc = &input.value().as<ayu::Document>();
    }, "Getting typed value from a resource");
    is(input.state(), LOADED, "Resource::value() automatically loads resource");
    is(input["foo"][1].get_as<int32>(), 4, "Value was generated properly (0)");
    is(input["bar"][1].get_as<std::string>(), "qux", "Value was generated properly (1)");

    throws<InvalidResourceState>([&]{ save(output); }, "save throws on unloaded resource");

    doc->delete_named("foo");
    doc->new_named<int32>("asdf", 51);

    doesnt_throw([&]{ rename(input, output); }, "rename");
    is(input.state(), UNLOADED, "Old resource is UNLOADED after renaming");
    is(output.state(), LOADED, "New resource is LOADED after renaming");
    is(&output.value().as<ayu::Document>(), doc, "Rename moves value without reconstructing it");

    doesnt_throw([&]{ save(output); }, "save");
    is(tree_from_file(resource_filename(output.name())), tree_from_string(
        "[ayu::Document {bar:[std::string qux] asdf:[int32 51] _next_id:0}]"
    ), "Resource was saved with correct contents");
    ok(source_exists(output), "source_exists returns true before deletion");
    doesnt_throw([&]{ remove_source(output); }, "remove_source");
    ok(!source_exists(output), "source_exists returns false after deletion");
    throws<OpenFailed>([&]{
        tree_from_file(resource_filename(output.name()));
    }, "Can't open file after calling remove_source");
    doesnt_throw([&]{ remove_source(output); }, "Can call remove_source twice");
    Location loc;
    doesnt_throw([&]{
        item_from_string(&loc, input.name().spec() + "#bar/1");
    }, "Can read location from tree");
    Reference ref;
    doesnt_throw([&]{
        ref = reference_from_location(loc);
    }, "reference_from_location");
    doesnt_throw([&]{
        is(ref.get_as<std::string>(), "qux", "reference_from_location got correct item");
    });
    doc = &output.value().as<ayu::Document>();
    ref = output["asdf"][1].address_as<int32>();
    doesnt_throw([&]{
        loc = reference_to_location(ref);
    });
    is(item_to_tree(&loc), tree_from_string("ayu-test:/test-output.ayu#asdf/1"), "reference_to_location works");
    doc->new_<Reference>(output["bar"][1]);
    doesnt_throw([&]{ save(output); }, "save with reference");
    doc->new_<int32*>(output["asdf"][1]);
    doesnt_throw([&]{ save(output); }, "save with pointer");
    is(tree_from_file(resource_filename(output.name())), tree_from_string(
        "[ayu::Document {bar:[std::string qux] asdf:[int32 51] _0:[ayu::Reference #bar/1] _1:[int32* #asdf/1] _next_id:2}]"
    ), "File was saved with correct reference as location");
    throws<OpenFailed>([&]{
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
        "Loading pointer with \"#\" for own file worked."
    );
    throws<UnloadWouldBreak>([&]{
        unload(input);
    }, "Can't unload resource when there are references to it");
    doesnt_throw([&]{
        unload(input2);
        unload(input);
    }, "Can unload if we unload the referring resource first");
    doesnt_throw([&]{
        load(rec1);
    }, "Can load resources with reference cycle");
    throws<UnloadWouldBreak>([&]{
        unload(rec1);
    }, "Can't unload part of a reference cycle 1");
    throws<UnloadWouldBreak>([&]{
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

    throws<UnacceptableResourceType>([&]{
        load("ayu-test:/wrongtype.ayu");
    }, "ResourceScheme::accepts_type rejects wrong type");

    done_testing();
});
#endif
