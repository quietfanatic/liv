#include "../location.h"

#include "../describe.h"
#include "../reference.h"
#include "tree-private.h"

namespace ayu {
namespace in {

enum LocationForm {
    KEY,
    INDEX
};

struct LocationData : RefCounted {
    uint8 form;
    uint16 length;  // Free space here, may as well use it
    Location parent;
    LocationData (uint8 f, Location p) : form(f), length(1 + p.length()), parent(p) { }
};

struct KeyLocation : LocationData {
    String key;
    KeyLocation (Location p, String&& k) :
        LocationData(KEY, p),
        key(k)
    { }
};
struct IndexLocation : LocationData {
    usize index;
    IndexLocation (Location p, usize i) :
        LocationData(INDEX, p),
        index(i)
    { }
};

void delete_LocationData (LocationData* p) {
    if (p->form == KEY) delete static_cast<KeyLocation*>(p);
    else delete static_cast<IndexLocation*>(p);
}

} using namespace in;

Location::Location (Location p, String&& k) : data(new KeyLocation(p, std::move(k))) { }
Location::Location (Location p, usize i) : data(new IndexLocation(p, i)) { }

const Location* Location::parent () const {
    return data ? &data->parent : null;
}
const String* Location::key () const {
    if (data && data->form == KEY) {
        return &static_cast<KeyLocation*>(data.p)->key;
    }
    else return null;
}
const usize* Location::index () const {
    if (data && data->form == INDEX) {
        return &static_cast<IndexLocation*>(data.p)->index;
    }
    else return null;
}

usize Location::length () const {
    return data ? data->length : 0;
}

bool operator == (const Location& a, const Location& b) {
    if (a.data == b.data) return true;
    if (!a.data || !b.data) return false;
    if (a.data->form != b.data->form) return false;
    if (a.data->length != b.data->length) return false;
    if (a.data->parent != b.data->parent) return false;
    if (a.data->form == KEY) {
        return static_cast<KeyLocation*>(a.data.p)->key
            == static_cast<KeyLocation*>(b.data.p)->key;
    }
    else {
        return static_cast<IndexLocation*>(a.data.p)->index
            == static_cast<IndexLocation*>(b.data.p)->index;
    }
}
// TODO: replace with this (slightly more efficient and idiomatic)
//bool operator == (const Location& a, const Location& b) {
//    if (a.data == b.data) return true;
//    if (!a.data || !b.data) return false;
//    if (a.data->length != b.data->length) return false;
//    if (a.data->form != b.data->form) return false;
//    switch (a.data->>form) {
//        case KEY:
//            auto ka = static_cast<KeyLocation*>(a.data.p);
//            auto kb = static_cast<KeyLocation*>(b.data.p);
//            if (ka->key != kb->key) return false;
//            break;
//        case INDEX:
//            auto ia = static_cast<IndexLocation*>(a.data.p);
//            auto ib = static_cast<IndexLocation*>(b.data.p);
//            if (ia->index != ib->index) return false;
//            break;
//    }
//     // Finally recurse
//    return a.data->parent == b.data->parent;
//}

} using namespace ayu;

static void location_to_array (Array& a, const Location& loc) {
    if (!loc.data) return;
    location_to_array(a, loc.data->parent);
    if (loc.data->form == KEY) {
        auto kloc = static_cast<KeyLocation*>(loc.data.p);
        a.emplace_back(Tree(kloc->key));
        return;
    }
    else {
        auto iloc = static_cast<IndexLocation*>(loc.data.p);
        a.emplace_back(Tree(iloc->index));
        return;
    }
}

AYU_DESCRIBE(ayu::Location,
    to_tree([](const Location& v){
        Array a;
        location_to_array(a, v);
        return Tree(std::move(a));
    }),
    from_tree([](Location& v, const Tree& t){
        if (t.form() != ARRAY) throw X::InvalidForm(&v, t);
        Location r;
        for (auto& e : t.data->as_known<Array>()) {
            switch (e.form()) {
                case STRING: r = Location(r, e.data->as_known<String>()); break;
                case NUMBER: r = Location(r, int64(e)); break;
                case ERROR: {
                    std::rethrow_exception(e.data->as_known<std::exception_ptr>());
                }
                default: {
                    throw X::GenericError("Location element is not string or integer");
                }
            }
        }
        v = std::move(r);
    }),
    length(value_func<usize>([](const Location& v){ return v.length(); })),
    elem_func([](Location& v, usize i){
        Location seg = v;
        for (usize j = i; j; j--) {
            if (auto p = seg.parent()) seg = *p;
            else return Reference();
        }
        if (!seg.data) return Reference();
        if (seg.data->form == KEY) {
            return Reference(&static_cast<KeyLocation*>(seg.data.p)->key);
        }
        else {
            return Reference(&static_cast<IndexLocation*>(seg.data.p)->index);
        }
    })
);

// TODO: tests
