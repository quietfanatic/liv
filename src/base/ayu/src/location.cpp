#include "../location.h"

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
    INDEX
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
    String key;
    KeyLocation (Location p, String&& k) :
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

void delete_LocationData (LocationData* p) {
    switch (p->form) {
        case ROOT: delete static_cast<RootLocation*>(p); break;
        case KEY: delete static_cast<KeyLocation*>(p); break;
        case INDEX: delete static_cast<IndexLocation*>(p); break;
        default: AYU_INTERNAL_UGUU();
    }
}

} using namespace in;

Location::Location (Resource res) :
    data(new RootLocation(std::move(res)))
{ }
Location::Location (Location p, String&& k) :
    data(new KeyLocation(p, std::move(k)))
{ *p.data; } // Segfault early if p is empty
Location::Location (Location p, usize i) :
    data(new IndexLocation(p, i))
{ *p.data; }

Location::Location (const IRI& iri) {
    Location self = Location(new RootLocation(iri.iri_without_fragment()));
    Str fragment = iri.fragment();
    if (fragment != "") {
        usize segment_start = 0;
        bool segment_is_string = false;
        for (usize i = 0; i < fragment.size()+1; i++) {
            switch (i == fragment.size() ? '/' : fragment[i]) {
                case '/': {
                    Str segment = fragment.substr(
                        segment_start, i - segment_start
                    );
                    if (segment_is_string) {
                        self = Location(self, iri::decode(segment));
                    }
                    else if (segment.size() == 0) {
                         // Ignore
                    }
                    else {
                        usize index;
                        auto [ptr, ec] = std::from_chars(
                            segment.begin(), segment.end(), index
                        );
                        if (ptr == 0) {
                            throw X::GenericError("Index segment too big?");
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

const Resource* Location::resource () const {
    if (data->form == ROOT) {
        return &static_cast<RootLocation*>(data.p)->resource;
    }
    else return null;
}

const Location* Location::parent () const {
    switch (data->form) {
        case ROOT: return null;
        case KEY: return &static_cast<KeyLocation*>(data.p)->parent;
        case INDEX: return &static_cast<IndexLocation*>(data.p)->parent;
        default: AYU_INTERNAL_UGUU();
    }
}
const String* Location::key () const {
    if (data->form == KEY) {
        return &static_cast<KeyLocation*>(data.p)->key;
    }
    else return null;
}
const usize* Location::index () const {
    if (data->form == INDEX) {
        return &static_cast<IndexLocation*>(data.p)->index;
    }
    else return null;
}

usize Location::length () const {
    if (!data) return 0;
    usize r = 0;
    for (const Location* l = this; l; l = l->parent()) {
        r += 1;
    }
    return r;
}

bool operator == (const Location& a, const Location& b) {
    if (a.data == b.data) return true;
    if (!a.data || !b.data) return false;
    if (a.data->form != b.data->form) return false;
    switch (a.data->form) {
        case ROOT: {
            auto aa = static_cast<RootLocation*>(a.data.p);
            auto bb = static_cast<RootLocation*>(b.data.p);
            return aa->resource == bb->resource;
        }
        case KEY: {
            auto aa = static_cast<KeyLocation*>(a.data.p);
            auto bb = static_cast<KeyLocation*>(b.data.p);
            return aa->key == bb->key && aa->parent == bb->parent;
        }
        case INDEX: {
            auto aa = static_cast<IndexLocation*>(a.data.p);
            auto bb = static_cast<IndexLocation*>(b.data.p);
            return aa->index == bb->index && aa->parent == bb->parent;
        }
        default: AYU_INTERNAL_UGUU();
    }
}

} using namespace ayu;

static void location_to_array (Array& a, const Location& loc) {
    if (!loc.data) return;
    switch (loc.data->form) {
        case ROOT: {
            auto rloc = static_cast<RootLocation*>(loc.data.p);
            a.emplace_back(item_to_tree(&rloc->resource));
            return;
        }
        case KEY: {
            auto kloc = static_cast<KeyLocation*>(loc.data.p);
            location_to_array(a, kloc->parent);
            a.emplace_back(Tree(kloc->key));
            return;
        }
        case INDEX: {
            auto iloc = static_cast<IndexLocation*>(loc.data.p);
            location_to_array(a, iloc->parent);
            a.emplace_back(Tree(iloc->index));
            return;
        }
        default: AYU_INTERNAL_UGUU();
    }
}

AYU_DESCRIBE(ayu::Location,
    to_tree([](const Location& v){
        Array a;
        location_to_array(a, v);
        return Tree(std::move(a));
    }),
    from_tree([](Location& v, const Tree& t){
        v = Location();
        if (t.form() == STRING) {
            if (auto res = current_resource()) {
                v = Location(IRI(
                    t.data->as_known<String>(),
                    res.name()
                ));
            }
            else {
                v = Location(IRI(t.data->as_known<String>()));
            }
            return;
        }
        if (t.form() != ARRAY) throw X::InvalidForm(&v, t);
        const Array& a = t.data->as_known<Array>();
        if (a.size() == 0) return;
        v = Location(Resource(Str(a[0])));
        for (usize i = 1; i < a.size(); i++) {
            switch (a[i].form()) {
                case STRING:
                    v = Location(v, a[i].data->as_known<String>());
                    break;
                case NUMBER:
                    v = Location(v, int64(a[i]));
                    break;
                case ERROR: {
                    std::rethrow_exception(
                        a[i].data->as_known<std::exception_ptr>()
                    );
                }
                default: {
                    throw X::GenericError("Location element is not string or integer");
                }
            }
        }
    })
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
    is(*l->key(), "bu/p", "String key with /");
    l = l->parent();
    is(*l->index(), 1u, "Index 1");
    l = l->parent();
    is(*l->key(), "bar", "String key");
    l = l->parent();
    is(*l->resource(), Resource("ayu-test:/"), "Resource root");
    ok(!l->parent());

    done_testing();
});
#endif
