#include "../document.h"

#include <cctype>

#include "../common.h"
#include "../describe.h"
#include "../reference.h"

namespace ayu {
namespace in {

static usize parse_numbered_name (Str name) {
    if (name.empty() || name[0] != '_') return -1;
    for (usize i = 1; i < name.size(); i++) {
        if (!isdigit(name[i])) return usize(-1);
    }
     // Assuming name is nul-terminated
     // TODO: use std::from_chars
    return atoll(name.data() + 1);
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

 // TODO: make sure item is aligned properly
struct DocumentItemHeader : DocumentLinks {
    usize id = 0;
    String* name_p;
    Type type;
    DocumentItemHeader (DocumentLinks* links, Type t, usize id) :
        DocumentLinks(links),
        id(id),
        name_p(null),
        type(t)
    { }
    DocumentItemHeader (DocumentLinks* links, Type t, usize id, String&& name) :
        DocumentLinks(links),
        id(id),
        name_p(new String(std::move(name))),
        type(t)
    { }
    ~DocumentItemHeader () {
        delete name_p;
    }
    Str name () {
        if (!name_p) {
            name_p = new String(print_numbered_name(id));
        }
        return *name_p;
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
            if (header->type) header->type.destroy(header->data());
            header->~DocumentItemHeader();
            free(header);
        }
    }
};

 // TODO: I think this can be replaced with a simple pointer to the header, no?
 // Because when we set the keys, we actually allocate the header, so it exists,
 // and when we're deserializing a DocumentItemRef, we realloc the header in the
 // second elem's setter, at which point we can update the pointer in this
 // DocumentItemRef.
struct DocumentItemRef {
    DocumentData* doc;
    String name;
     // This is gonna be all sorts of inefficient, but we'll be able to
     // optimize it later if we need to by putting a temporary map in
     // DocumentData, or by caching the pointer here and invalidating it
     // with a version number.
     //
     // Or just do the above TODO.
    DocumentItemHeader* header () const {
        for (auto link = doc->items.next; link != &doc->items; link = link->next) {
            auto h = static_cast<DocumentItemHeader*>(link);
            if (h->name() == name) return h;
        }
        return null;
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

void* Document::allocate_named (Type t, Str name) {
    usize id = parse_numbered_name(name);

    if (name.empty()) throw X::DocumentInvalidName(String(name));
    if (id == usize(-1) && name[0] == '_') throw X::DocumentInvalidName(String(name));
    auto ref = DocumentItemRef{data, String(name)};
    if (ref.header()) throw X::DocumentDuplicateName(String(name));

    if (id == usize(-1)) {
        auto p = malloc(sizeof(DocumentItemHeader) + (t ? t.cpp_size() : 0));
        auto header = new (p) DocumentItemHeader(&data->items, t, id, String(name));
        return header+1;
    }
    else { // Actually a numbered item
        if (id > data->next_id + 10000) throw X::GenericError("Unreasonable growth of next_id"s);
        if (id >= data->next_id) data->next_id = id + 1;
        auto p = malloc(sizeof(DocumentItemHeader) + (t ? t.cpp_size() : 0));
        auto header = new (p) DocumentItemHeader(&data->items, t, id, String(name));
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
    if (header->type) header->type.destroy(p);
    header->~DocumentItemHeader();
    free(header);
}

void Document::delete_named (Str name) {
    auto ref = DocumentItemRef{data, String(name)};
    if (auto header = ref.header()) {
        if (header->type) header->type.destroy(header->data());
        header->~DocumentItemHeader();
        free(header);
        return;
    }
    else throw X::DocumentDeleteMissing(String(name));
}

void Document::deallocate (void* p) {
    auto header = (DocumentItemHeader*)p - 1;
    header->~DocumentItemHeader();
    free(header);
}

} using namespace ayu;

AYU_DESCRIBE(ayu::Document,
    keys(mixed_funcs<std::vector<Str>>(
        [](const ayu::Document& v){
            std::vector<Str> r;
            for (auto link = v.data->items.next; link != &v.data->items; link = link->next) {
                r.emplace_back(static_cast<DocumentItemHeader*>(link)->name());
            }
            r.emplace_back("_next_id"sv);
            return r;
        },
        [](ayu::Document& v, const std::vector<Str>& ks){
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
     // DocumentItemHeader has no type), we don't want to allow serializing it.
    elems(
        elem(value_funcs<Type>(
            [](const DocumentItemRef& v){
                return v.header()->type;
            },
            [](DocumentItemRef& v, Type t){
                if (auto header = v.header()) {
                    if (header->type) {
                        header->type.destroy(header->data());
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

AYU_DESCRIBE(ayu::X::DocumentError,
    delegate(base<X::Error>())
)

AYU_DESCRIBE(ayu::X::DocumentInvalidName,
    elems(
        elem(base<X::DocumentError>(), inherit),
        elem(&X::DocumentInvalidName::name)
    )
)
AYU_DESCRIBE(ayu::X::DocumentDuplicateName,
    elems(
        elem(base<X::DocumentError>(), inherit),
        elem(&X::DocumentDuplicateName::name)
    )
)
AYU_DESCRIBE(ayu::X::DocumentDeleteWrongType,
    elems(
        elem(base<X::DocumentError>(), inherit),
        elem(&X::DocumentDeleteWrongType::existing),
        elem(&X::DocumentDeleteWrongType::deleted_as)
    )
)
AYU_DESCRIBE(ayu::X::DocumentDeleteNotOwned,
    elems(elem(base<X::DocumentError>(), inherit))
)
AYU_DESCRIBE(ayu::X::DocumentDeleteMissing,
    elems(
        elem(base<X::DocumentError>(), inherit),
        elem(&X::DocumentDeleteMissing::name)
    )
)
