#include "../document.h"

#include <cctype>

#include "../common.h"
#include "../describe.h"
#include "../reference.h"

namespace ayu {
namespace in {

static usize parse_numbered_name (OldStr name) {
    if (name.empty() || name[0] != '_') return -1;
    for (usize i = 1; i < name.size(); i++) {
        if (!isdigit(name[i])) return usize(-1);
    }
     // Assuming name is nul-terminated
     // TODO: use std::from_chars
    return atoll(name.data() + 1);
}
static std::string print_numbered_name (usize id) {
    return old_cat('_', id);
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
    std::string* name_p;
    Type type;
    DocumentItemHeader (DocumentLinks* links, Type t, usize id) :
        DocumentLinks(links),
        id(id),
        name_p(null),
        type(t)
    { }
    DocumentItemHeader (DocumentLinks* links, Type t, usize id, std::string&& name) :
        DocumentLinks(links),
        id(id),
        name_p(new std::string(std::move(name))),
        type(t)
    { }
    ~DocumentItemHeader () {
        delete name_p;
    }
    OldStr name () {
        if (!name_p) {
            name_p = new std::string(print_numbered_name(id));
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

struct DocumentItemRef {
    DocumentItemHeader* header;
    DocumentItemRef (DocumentData* doc, OldStr name) {
        for (auto link = doc->items.next; link != &doc->items; link = link->next) {
            auto h = static_cast<DocumentItemHeader*>(link);
            if (h->name() == name) {
                header = h;
                return;
            }
        }
        header = null;
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

void* Document::allocate_named (Type t, OldStr name) {
    usize id = parse_numbered_name(name);

    if (name.empty()) throw X<DocumentInvalidName>(std::string(name));
    if (id == usize(-1) && name[0] == '_') {
        throw X<DocumentInvalidName>(std::string(name));
    }
    auto ref = DocumentItemRef(data, std::string(name));
    if (ref.header) throw X<DocumentDuplicateName>(std::string(name));

    if (id == usize(-1)) {
        auto p = malloc(sizeof(DocumentItemHeader) + (t ? t.cpp_size() : 0));
        auto header = new (p) DocumentItemHeader(&data->items, t, id, std::string(name));
        return header+1;
    }
    else { // Actually a numbered item
        if (id > data->next_id + 10000) throw X<GenericError>("Unreasonable growth of next_id"s);
        if (id >= data->next_id) data->next_id = id + 1;
        auto p = malloc(sizeof(DocumentItemHeader) + (t ? t.cpp_size() : 0));
        auto header = new (p) DocumentItemHeader(&data->items, t, id, std::string(name));
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
    never();
    we_good:;
#endif
    auto header = (DocumentItemHeader*)p - 1;
    if (header->type != t) throw X<DocumentDeleteWrongType>(header->type, t);
    if (header->type) header->type.destroy(p);
    header->~DocumentItemHeader();
    free(header);
}

void Document::delete_named (OldStr name) {
    auto ref = DocumentItemRef(data, std::string(name));
    if (ref.header) {
        if (ref.header->type) {
            ref.header->type.destroy(ref.header->data());
        }
        ref.header->~DocumentItemHeader();
        free(ref.header);
        return;
    }
    else throw X<DocumentDeleteMissing>(std::string(name));
}

void Document::deallocate (void* p) {
    auto header = (DocumentItemHeader*)p - 1;
    header->~DocumentItemHeader();
    free(header);
}

} using namespace ayu;

AYU_DESCRIBE(ayu::Document,
    keys(mixed_funcs<std::vector<OldStr>>(
        [](const ayu::Document& v){
            std::vector<OldStr> r;
            for (auto link = v.data->items.next; link != &v.data->items; link = link->next) {
                r.emplace_back(static_cast<DocumentItemHeader*>(link)->name());
            }
            r.emplace_back("_next_id"sv);
            return r;
        },
        [](ayu::Document& v, const std::vector<OldStr>& ks){
            v.data->~DocumentData();
            new (v.data) DocumentData;
            for (auto& k : ks) {
                if (k == "_next_id"sv) continue;
                v.allocate_named(Type(), k);
            }
        }
    )),
    attr_func([](ayu::Document& v, OldStr k){
        if (k == "_next_id"sv) {
            return Reference(&v.data->next_id);
        }
        else {
            auto ref = DocumentItemRef(v.data, std::string(k));
            if (ref.header) {
                return Reference(
                    v, variable(std::move(ref), pass_through_addressable)
                );
            }
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
                return v.header->type;
            },
            [](DocumentItemRef& v, Type t){
                if (v.header->type) {
                    v.header->type.destroy(v.header->data());
                }
                 // This is a very bad idea which should work.
                 // (cast to void* to silence warning)
                 // (note: if instead we call ~DocumentItemHeader without
                 //  first cleaning prev and next, it will reorder items in
                 //  the document.)
                v.header = (DocumentItemHeader*)realloc(
                    (void*)v.header, sizeof(DocumentItemHeader) + (t ? t.cpp_size() : 0)
                );
                v.header->prev->next = v.header;
                v.header->next->prev = v.header;
                v.header->type = t;
                if (v.header->type) {
                    v.header->type.default_construct(v.header->data());
                }
            }
        )),
        elem(reference_func([](DocumentItemRef& v){
            if (v.header->type) {
                return Reference(v.header->data(), v.header->type);
            }
            else return Reference();
        }))
    )
)

AYU_DESCRIBE(ayu::DocumentError,
    delegate(base<Error>())
)

AYU_DESCRIBE(ayu::DocumentInvalidName,
    elems(
        elem(base<DocumentError>(), inherit),
        elem(&DocumentInvalidName::name)
    )
)
AYU_DESCRIBE(ayu::DocumentDuplicateName,
    elems(
        elem(base<DocumentError>(), inherit),
        elem(&DocumentDuplicateName::name)
    )
)
AYU_DESCRIBE(ayu::DocumentDeleteWrongType,
    elems(
        elem(base<DocumentError>(), inherit),
        elem(&DocumentDeleteWrongType::existing),
        elem(&DocumentDeleteWrongType::deleted_as)
    )
)
AYU_DESCRIBE(ayu::DocumentDeleteMissing,
    elems(
        elem(base<DocumentError>(), inherit),
        elem(&DocumentDeleteMissing::name)
    )
)
