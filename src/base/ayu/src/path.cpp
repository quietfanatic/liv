#include "../path.h"

#include "../describe.h"
#include "../reference.h"
#include "../serialize.h"
#include "tree-private.h"

namespace ayu {
namespace in {

enum PathForm {
    KEY,
    INDEX
};

struct PathData : RefCounted {
    uint8 form;
    uint16 length;  // Free space here, may as well use it
    Path parent;
    PathData (uint8 f, Path p) : form(f), length(1 + p.length()), parent(p) { }
};

struct KeyPath : PathData {
    String key;
    KeyPath (Path p, String&& k) :
        PathData(KEY, p),
        key(k)
    { }
};
struct IndexPath : PathData {
    usize index;
    IndexPath (Path p, usize i) :
        PathData(INDEX, p),
        index(i)
    { }
};

void delete_PathData (PathData* p) {
    if (p->form == KEY) delete static_cast<KeyPath*>(p);
    else delete static_cast<IndexPath*>(p);
}

} using namespace in;

Path::Path (Path p, String&& k) : data(new KeyPath(p, std::move(k))) { }
Path::Path (Path p, usize i) : data(new IndexPath(p, i)) { }

const Path* Path::parent () const {
    return data ? &data->parent : null;
}
const String* Path::key () const {
    if (data && data->form == KEY) {
        return &static_cast<KeyPath*>(data.p)->key;
    }
    else return null;
}
const usize* Path::index () const {
    if (data && data->form == INDEX) {
        return &static_cast<IndexPath*>(data.p)->index;
    }
    else return null;
}

usize Path::length () const {
    return data ? data->length : 0;
}

bool operator == (const Path& a, const Path& b) {
    if (a.data == b.data) return true;
    if (!a.data || !b.data) return false;
    if (a.data->form != b.data->form) return false;
    if (a.data->length != b.data->length) return false;
    if (a.data->parent != b.data->parent) return false;
    if (a.data->form == KEY) {
        return static_cast<KeyPath*>(a.data.p)->key
            == static_cast<KeyPath*>(b.data.p)->key;
    }
    else {
        return static_cast<IndexPath*>(a.data.p)->index
            == static_cast<IndexPath*>(b.data.p)->index;
    }
}
// TODO: replace with this (slightly more efficient and idiomatic)
//bool operator == (const Path& a, const Path& b) {
//    if (a.data == b.data) return true;
//    if (!a.data || !b.data) return false;
//    if (a.data->length != b.data->length) return false;
//    if (a.data->form != b.data->form) return false;
//    switch (a.data->>form) {
//        case KEY:
//            auto ka = static_cast<KeyPath*>(a.data.p);
//            auto kb = static_cast<KeyPath*>(b.data.p);
//            if (ka->key != kb->key) return false;
//            break;
//        case INDEX:
//            auto ia = static_cast<IndexPath*>(a.data.p);
//            auto ib = static_cast<IndexPath*>(b.data.p);
//            if (ia->index != ib->index) return false;
//            break;
//    }
//     // Finally recurse
//    return a.data->parent == b.data->parent;
//}

} using namespace ayu;

static void p2a (Array& a, const Path& p) {
    if (!p.data) return;
    p2a(a, p.data->parent);
    if (p.data->form == KEY) {
        auto kp = static_cast<KeyPath*>(p.data.p);
        a.emplace_back(Tree(kp->key));
        return;
    }
    else {
        auto ip = static_cast<IndexPath*>(p.data.p);
        a.emplace_back(Tree(ip->index));
        return;
    }
}

AYU_DESCRIBE(ayu::Path,
    to_tree([](const Path& v){
        Array a;
        p2a(a, v);
        return Tree(std::move(a));
    }),
    from_tree([](Path& v, const Tree& t){
        if (t.form() != ARRAY) throw X::InvalidForm(&v, t);
        Path r;
        for (auto& e : t.data->as_known<Array>()) {
            switch (e.form()) {
            case STRING: r = Path(r, e.data->as_known<String>()); break;
            case NUMBER: r = Path(r, int64(e)); break;
            default: throw X::GenericError("Path element is not string or integer");
            }
        }
        v = std::move(r);
    }),
    length(value_func<usize>([](const Path& v){ return v.length(); })),
    elem_func([](Path& v, usize i){
        Path seg = v;
        for (usize j = i; j; j--) {
            if (auto p = seg.parent()) seg = *p;
            else return Reference();
        }
        if (!seg.data) return Reference();
        if (seg.data->form == KEY) {
            return Reference(&static_cast<KeyPath*>(seg.data.p)->key);
        }
        else {
            return Reference(&static_cast<IndexPath*>(seg.data.p)->index);
        }
    })
);

// TODO: tests
