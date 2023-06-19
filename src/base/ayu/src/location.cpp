#include "location-private.h"

#include <charconv>
#include "../describe.h"
#include "../reference.h"
#include "../resource.h"
#include "char-cases-private.h"
#include "tree-private.h"

namespace ayu {
namespace in {

enum LocationForm {
    ROOT,
    KEY,
    INDEX,
     // Internal for lazy error throwing
    ERROR_LOC,
};

struct LocationData : RefCounted {
    uint8 form;
    LocationData (uint8 f) : form(f) { }
};

struct RootLocation : LocationData {
    Resource resource;
    RootLocation (Resource&& res) :
        LocationData(ROOT), resource(res)
    { }
};

struct KeyLocation : LocationData {
    Location parent;
    std::string key;
    KeyLocation (Location p, std::string&& k) :
        LocationData(KEY), parent(p), key(k)
    { }
};
struct IndexLocation : LocationData {
    Location parent;
    usize index;
    IndexLocation (Location p, usize i) :
        LocationData(INDEX), parent(p), index(i)
    { }
};
struct ErrorLocation : LocationData {
    std::exception_ptr error;
    ErrorLocation (std::exception_ptr&& e) :
        LocationData(ERROR_LOC), error(std::move(e)) { }
};

Location make_error_location (std::exception_ptr&& e) {
    return Location(new ErrorLocation(std::move(e)));
}

[[noreturn]]
static void rethrow (LocationRef l) {
    expect(l->data->form == ERROR_LOC);
    std::rethrow_exception(
        static_cast<ErrorLocation*>(l->data.p)->error
    );
}

void delete_LocationData (LocationData* p) {
    switch (p->form) {
        case ROOT: delete static_cast<RootLocation*>(p); break;
        case KEY: delete static_cast<KeyLocation*>(p); break;
        case INDEX: delete static_cast<IndexLocation*>(p); break;
         // Okay to delete without throwing
        case ERROR_LOC: delete static_cast<ErrorLocation*>(p); break;
        default: never();
    }
}

} using namespace in;

Location::Location (Resource res) :
    data(new RootLocation(std::move(res)))
{ }
Location::Location (LocationRef p, std::string&& k) :
    data(new KeyLocation(p, std::move(k)))
{ }
Location::Location (LocationRef p, usize i) :
    data(new IndexLocation(p, i))
{ }

Location::Location (const IRI& iri) {
    if (!iri) return;
    Location self = Location(new RootLocation(iri.iri_without_fragment()));
    OldStr fragment = iri.fragment();
    if (!fragment.empty()) {
        usize segment_start = 0;
        bool segment_is_string = false;
        for (usize i = 0; i < fragment.size()+1; i++) {
            switch (i == fragment.size() ? '/' : fragment[i]) {
                case '/': {
                    OldStr segment = fragment.substr(
                        segment_start, i - segment_start
                    );
                    if (segment_is_string) {
                        self = Location(self, iri::decode(segment));
                    }
                    else if (segment.size() == 0) {
                         // Ignore
                    }
                    else {
                        usize index = -1;
                        auto [ptr, ec] = std::from_chars(
                            segment.begin(), segment.end(), index
                        );
                        if (ptr == 0) {
                            throw X<GenericError>("Index segment too big?"s);
                        }
                        self = Location(self, index);
                    }
                    segment_start = i+1;
                    segment_is_string = false;
                    break;
                }
                case '\'': {
                    if (i == segment_start && !segment_is_string) {
                        segment_start = i+1;
                    }
                    segment_is_string = true;
                    break;
                }
                case ANY_DECIMAL_DIGIT: break;
                default: segment_is_string = true; break;
            }
        }
    }
    *this = std::move(self);
}

IRI Location::as_iri () const {
    if (!*this) return IRI();
    std::string fragment;
    for (const Location* l = this;; l = l->parent()) {
        expect(l);
        if (!l->data) {
            if (fragment.empty()) return IRI("anonymous-item:"sv);
            else return IRI(old_cat("anonymous-item:"sv, "#", fragment));
        }
        else switch (l->data->form) {
            case ROOT: {
                const IRI& base = static_cast<RootLocation*>(
                    l->data.p
                )->resource.name();
                if (fragment.empty()) return base;
                else return IRI(old_cat('#', fragment), base);
            }
            case KEY: {
                OldStr key = static_cast<KeyLocation*>(
                    l->data.p
                )->key;
                std::string segment;
                if (key.empty() || key[0] == '\'' || std::isdigit(key[0])) {
                    segment = old_cat('\'', iri::encode(key));
                }
                else segment = iri::encode(key);
                if (fragment.empty()) fragment = segment;
                else fragment = old_cat(segment, '/', fragment);
                break;
            }
            case INDEX: {
                usize index = static_cast<IndexLocation*>(
                    l->data.p
                )->index;
                if (fragment.empty()) fragment = old_cat(index);
                else fragment = old_cat(index, '/', fragment);
                break;
            }
            case ERROR_LOC: rethrow(*l);
            default: never();
        }
    }
}

const Resource* Location::resource () const {
    if (data && data->form == ROOT) {
        return &static_cast<RootLocation*>(data.p)->resource;
    }
    else return null;
}

const Location* Location::parent () const {
    if (!data) return null;
    switch (data->form) {
        case ROOT: return null;
        case KEY: return &static_cast<KeyLocation*>(data.p)->parent;
        case INDEX: return &static_cast<IndexLocation*>(data.p)->parent;
        case ERROR_LOC: rethrow(*this);
        default: never();
    }
}
const std::string* Location::key () const {
    if (!data) return null;
    switch (data->form) {
        case KEY: return &static_cast<KeyLocation*>(data.p)->key;
        case ERROR_LOC: rethrow(*this);
        default: return null;
    }
}
const usize* Location::index () const {
    if (!data) return null;
    switch (data->form) {
        case INDEX: return &static_cast<IndexLocation*>(data.p)->index;
        case ERROR_LOC: rethrow(*this);
        default: return null;
    }
}

usize Location::length () const {
    if (!data) return 0;
    usize r = 0;
    for (const Location* l = this; l; l = l->parent()) {
        r += 1;
    }
    return r;
}

const Resource* Location::root_resource () const {
    if (!data) return null;
    switch (data->form) {
        case ROOT: return &static_cast<RootLocation*>(data.p)->resource;
        case INDEX: return static_cast<IndexLocation*>(data.p)->parent.root_resource();
        case KEY: return static_cast<KeyLocation*>(data.p)->parent.root_resource();
        case ERROR_LOC: rethrow(*this);
        default: never();
    }
}

bool operator == (LocationRef a, LocationRef b) {
    if (a->data == b->data) return true;
    if (!a->data || !b->data) return false;
    if (a->data->form == ERROR_LOC) rethrow(a);
    if (b->data->form == ERROR_LOC) rethrow(b);
    if (a->data->form != b->data->form) return false;
    switch (a->data->form) {
        case ROOT: {
            auto aa = static_cast<RootLocation*>(a->data.p);
            auto bb = static_cast<RootLocation*>(b->data.p);
            return aa->resource == bb->resource;
        }
        case KEY: {
            auto aa = static_cast<KeyLocation*>(a->data.p);
            auto bb = static_cast<KeyLocation*>(b->data.p);
            return aa->key == bb->key && aa->parent == bb->parent;
        }
        case INDEX: {
            auto aa = static_cast<IndexLocation*>(a->data.p);
            auto bb = static_cast<IndexLocation*>(b->data.p);
            return aa->index == bb->index && aa->parent == bb->parent;
        }
        default: never();
    }
}

Reference reference_from_location (Location loc) {
    if (!loc) return Reference();
    if (auto parent = loc.parent()) {
        if (auto key = loc.key()) {
            return reference_from_location(*parent).attr(*key);
        }
        else if (auto index = loc.index()) {
            return reference_from_location(*parent).elem(*index);
        }
        else never();
    }
    else if (auto res = loc.resource()) {
        return res->ref();
    }
    else never();
}

} using namespace ayu;

AYU_DESCRIBE(ayu::Location,
    delegate(mixed_funcs<IRI>(
        [](const Location& v){
            return v.as_iri();
        },
        [](Location& v, const IRI& m){
            v = Location(m);
        }
    ))
);

// TODO: tests

#ifndef TAP_DISABLE_TESTS
#include "test-environment-private.h"

static tap::TestSet tests ("base/ayu/location", []{
    using namespace tap;

    test::TestEnvironment env;

    Location loc = Location(IRI("ayu-test:/#bar/1/bu%2Fp//33/0/'3/''/'//"));
    const Location* l = &loc;
    is(*l->key(), "", "Empty key");
    l = l->parent();
    is(*l->key(), "'", "Key with apostrophe");
    l = l->parent();
    is(*l->key(), "3", "Number-like key");
    l = l->parent();
    is(*l->index(), 0u, "Index 0");
    l = l->parent();
    is(*l->index(), 33u, "Index 33");
    l = l->parent();
    is(*l->key(), "bu/p", "std::string key with /");
    l = l->parent();
    is(*l->index(), 1u, "Index 1");
    l = l->parent();
    is(*l->key(), "bar", "std::string key");
    l = l->parent();
    is(*l->resource(), Resource("ayu-test:/"), "Resource root");
    ok(!l->parent());

    done_testing();
});
#endif
