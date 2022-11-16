#include "../document.h"

#include <cctype>

#include "../common.h"
#include "../describe.h"
#include "../reference.h"

namespace ayu {
namespace in {

static usize parse_numbered_name (const String& name) {
    if (name.empty() || name[0] != '_') return -1;
    for (usize i = 1; i < name.size(); i++) {
        if (!isdigit(name[i])) return usize(-1);
    }
    return atoll(name.c_str() + 1);
}
static String print_numbered_name (usize id) {
    return cat('_', id);
}

struct DocumentLinks {
    DocumentLinks* prev;
    DocumentLinks* next;
    DocumentLinks () : prev(this), next(this) { }
    DocumentLinks (DocumentLinks* o) :
        prev(o->prev),
        next(o)
    {
        o->prev->next = this;
        o->prev = this;
    }
    ~DocumentLinks () {
        prev->next = next;
        next->prev = prev;
    }
};

struct DocumentItemHeader : DocumentLinks {
    union {
        String* name_p;
        usize idx2p1;
    };
    Type type;
    DocumentItemHeader (DocumentLinks* links, Type t, usize id) :
        DocumentLinks(links),
        idx2p1(id << 1 | 1),
        type(t)
    { }
    DocumentItemHeader (DocumentLinks* links, Type t, String&& name) :
        DocumentLinks(links),
        name_p(new String(std::move(name))),
        type(t)
    { }
    ~DocumentItemHeader () {
        if (has_name()) delete name_p;
    }
    bool has_name () {
        return !(idx2p1 & 1);
    }
    String name () {
        if (has_name()) return *name_p;
        else return print_numbered_name(idx2p1 >> 1);
    }
    usize id () {
        if (has_name()) return usize(-1);
        else return idx2p1 >> 1;
    }
    Mu* data () {
        return (Mu*)(this + 1);
    }
};

struct DocumentData {
    DocumentLinks items;
    usize next_id = 0;
    ~DocumentData () {
        while (items.next != &items) {
            auto header = static_cast<DocumentItemHeader*>(items.next);
            if (header->type) header->type.destruct(*header->data());
            header->~DocumentItemHeader();
            free(header);
        }
    }
};

struct DocumentItemRef {
    DocumentData* doc;
    String name;
     // This is gonna be all sorts of inefficient, but we'll be able to
     //  optimize it later if we need to by putting a temporary map in
     //  DocumentData, or by caching the pointer here and invalidating it
     //  with a version number.
    DocumentItemHeader* header () const {
        usize id = parse_numbered_name(name);
        if (id == usize(-1)) {
            for (auto link = doc->items.next; link != &doc->items; link = link->next) {
                auto h = static_cast<DocumentItemHeader*>(link);
                if (h->has_name() && h->name() == name) return h;
            }
            return null;
        }
        else {
            for (auto link = doc->items.next; link != &doc->items; link = link->next) {
                auto h = static_cast<DocumentItemHeader*>(link);
                if (h->id() == id) return h;
            }
            return null;
        }
    }
};

} using namespace in;

Document::Document () : data(new DocumentData) { }
Document::~Document () { delete data; }

void* Document::allocate (Type t) {
    auto id = data->next_id++;
    auto p = malloc(sizeof(DocumentItemHeader) + (t ? t.cpp_size() : 0));
    auto header = new (p) DocumentItemHeader(&data->items, t, id);
    return header+1;
}

void* Document::allocate_named (Type t, const String& name) {
    usize id = parse_numbered_name(name);

    if (name.empty()) throw X::DocumentInvalidName(name);
    if (id == usize(-1) && name[0] == '_') throw X::DocumentInvalidName(name);
    auto ref = DocumentItemRef{data, name};
    if (ref.header()) throw X::DocumentDuplicateName(name);

    if (id == usize(-1)) {
        auto p = malloc(sizeof(DocumentItemHeader) + (t ? t.cpp_size() : 0));
        auto header = new (p) DocumentItemHeader(&data->items, t, String(name));
        return header+1;
    }
    else { // Actually a numbered item
        if (id > data->next_id + 10000) throw X::GenericError("Unreasonable growth of next_id"s);
        if (id >= data->next_id) data->next_id = id + 1;
        auto p = malloc(sizeof(DocumentItemHeader) + (t ? t.cpp_size() : 0));
        auto header = new (p) DocumentItemHeader(&data->items, t, id);
        return header+1;
    }
}

void Document::delete_ (Type t, Mu* p) {
#ifndef NDEBUG
     // Check that the pointer belongs to this document
    for (auto link = data->items.next; link != &data->items; link = link->next) {
        auto header = static_cast<DocumentItemHeader*>(link);
        if (header->data() == p) goto we_good;
    }
    throw X::DocumentDeleteNotOwned();
    we_good:;
#endif
    auto header = (DocumentItemHeader*)p - 1;
    if (header->type != t) throw X::DocumentDeleteWrongType(header->type, t);
    if (header->type) header->type.destruct(*p);
    header->~DocumentItemHeader();
    free(header);
}

void Document::delete_named (const String& name) {
    auto ref = DocumentItemRef{data, name};
    if (auto header = ref.header()) {
        if (header->type) header->type.destruct(*header->data());
        header->~DocumentItemHeader();
        free(header);
        return;
    }
    else throw X::DocumentDeleteMissing(name);
}

void Document::deallocate (void* p) {
    auto header = (DocumentItemHeader*)p - 1;
    header->~DocumentItemHeader();
    free(header);
}

} using namespace ayu;

AYU_DESCRIBE(ayu::Document,
    keys(mixed_funcs<std::vector<String>>(
        [](const ayu::Document& v){
            std::vector<String> r;
            for (auto link = v.data->items.next; link != &v.data->items; link = link->next) {
                r.emplace_back(static_cast<DocumentItemHeader*>(link)->name());
            }
            r.emplace_back("_next_id"s);
            return r;
        },
        [](ayu::Document& v, const std::vector<String>& ks){
            v.data->~DocumentData();
            new (v.data) DocumentData;
            for (auto& k : ks) {
                if (k == "_next_id"sv) continue;
                v.allocate_named(Type(), k);
            }
        }
    )),
    attr_func([](ayu::Document& v, Str k){
        if (k == "_next_id"sv) {
            return Reference(&v.data->next_id);
        }
        else {
            auto ref = DocumentItemRef{v.data, String(k)};
            if (ref.header()) return Reference(v, variable(std::move(ref)));
            else return Reference();
        }
    })
)

AYU_DESCRIBE(ayu::in::DocumentItemRef,
     // Although nullishness is a valid state for DocumentItemRef (meaning the
     //  DocumentItemHeader has no type), we don't want to allow serializing it.
    elems(
        elem(value_funcs<Type>(
            [](const DocumentItemRef& v){
                return v.header()->type;
            },
            [](DocumentItemRef& v, Type t){
                if (auto header = v.header()) {
                    if (header->type) {
                        header->type.destruct(*header->data());
                    }
                     // This is a very bad idea which should work.
                     // (cast to void* to silence warning)
                     // (note: if instead we call ~DocumentItemHeader without
                     //  first cleaning prev and next, it will reorder items in
                     //  the document.)
                    header = (DocumentItemHeader*)realloc(
                        (void*)header, sizeof(DocumentItemHeader) + (t ? t.cpp_size() : 0)
                    );
                    header->prev->next = header;
                    header->next->prev = header;
                    header->type = t;
                    if (header->type) header->type.default_construct(header + 1);
                }
            }
        )),
        elem(reference_func([](DocumentItemRef& v){
            auto header = v.header();
            if (header->type) {
                return Reference(header->type, header->data());
            }
            else return Reference();
        }, anchored_to_grandparent))
    )
)

AYU_DESCRIBE(ayu::X::DocumentInvalidName,
    delegate(base<X::DocumentError>()),
    elems( elem(&X::DocumentInvalidName::name) )
)
AYU_DESCRIBE(ayu::X::DocumentDuplicateName,
    delegate(base<X::DocumentError>()),
    elems( elem(&X::DocumentDuplicateName::name) )
)
AYU_DESCRIBE(ayu::X::DocumentDeleteWrongType,
    delegate(base<X::DocumentError>()),
    elems(
        elem(&X::DocumentDeleteWrongType::existing),
        elem(&X::DocumentDeleteWrongType::deleted_as)
    )
)
AYU_DESCRIBE(ayu::X::DocumentDeleteNotOwned,
    delegate(base<X::DocumentError>())
)
AYU_DESCRIBE(ayu::X::DocumentDeleteMissing,
    delegate(base<X::DocumentError>()),
    elems( elem(&X::DocumentDeleteMissing::name) )
)
