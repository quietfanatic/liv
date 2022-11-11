#include "../location.h"

#include "../describe.h"
#include "../reference.h"
#include "../resource.h"
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
            a.emplace_back(Tree(rloc->resource.name().spec()));
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
