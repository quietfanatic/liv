#include "serialize-private.h"

#include <cassert>
#include "../describe.h"
#include "../parse.h"
#include "resource-private.h"

namespace ayu {
using namespace in;

///// TO_TREE
namespace in {
    static int64 diagnostic_serialization = 0;
}
DiagnosticSerialization::DiagnosticSerialization () {
    diagnostic_serialization += 1;
}
DiagnosticSerialization::~DiagnosticSerialization () {
    diagnostic_serialization -= 1;
    assert(diagnostic_serialization >= 0);
}

Tree in::inner_to_tree (
    const DescriptionPrivate* desc, const Mu& item, TempLocation* loc
) {
    try {
        if (auto to_tree = desc->to_tree()) {
            return to_tree->f(item);
        }
        if (auto values = desc->values()) {
            for (uint i = 0; i < values->n_values; i++) {
                Tree r = values->value(i)->value_to_tree(values, item);
                if (r.has_value()) return r;
            }
        }
        switch (desc->preference()) {
            case Description::PREFER_OBJECT: {
                Object o;
                auto ref = Reference(desc, &item);
                 // TODO: inner this too
                item_read_keys(ref, [&](const std::vector<Str>& ks){
                    for (auto& k : ks) {
                        Reference attr = item_attr(ref, k);
                        if (attr.readonly()) continue;
                        auto attr_desc = DescriptionPrivate::get(attr.type());
                        auto attr_loc = KeyTempLocation(loc, k);
                        attr.read([&](const Mu& v){
                            o.emplace_back(k, inner_to_tree(attr_desc, v, &attr_loc));
                        });
                    }
                });
                return Tree(std::move(o));
            }
            case Description::PREFER_ARRAY: {
                Array a;
                auto ref = Reference(desc, &item);
                usize l = item_get_length(ref);
                for (usize i = 0; i < l; i++) {
                    Reference elem = item_elem(ref, i);
                    if (elem.readonly()) continue;
                    auto elem_desc = DescriptionPrivate::get(elem.type());
                    auto elem_loc = IndexTempLocation(loc, i);
                    elem.read([&](const Mu& v){
                        a.emplace_back(inner_to_tree(elem_desc, v, &elem_loc));
                    });
                }
                return Tree(std::move(a));
            }
            default: {
                if (auto acr = desc->delegate_acr()) {
                    auto del_desc = DescriptionPrivate::get(acr->type(&item));
                    Tree r;
                    acr->read(item, [&](const Mu& v){
                        r = inner_to_tree(del_desc, v, loc);
                    });
                    return r;
                }
                else if (desc->values()) {
                    throw X::NoNameForValue(make_permanent(loc));
                }
                else throw X::CannotToTree(make_permanent(loc));
            }
        }
    }
    catch (const X::Error& e) {
        if (diagnostic_serialization) {
            return Tree(new TreeDataT<std::exception_ptr>(
                std::current_exception()
            ));
        }
        else throw;
    }
}
Tree item_to_tree (const Reference& item) {
    auto desc = DescriptionPrivate::get(item.type());
     // TODO: Make a current-item: scheme
    auto loc = RootTempLocation(Resource());
    Tree r;
    item.read([&](const Mu& v){
        r = inner_to_tree(desc, v, &loc);
    });
    return r;
}

///// FROM_TREE

void in::do_swizzles () {
    while (!swizzle_ops.empty()) {
        auto swizzles = std::move(swizzle_ops);
        for (auto& op : swizzles) {
            PushCurrentResource p (op.current_resource);
            op.item.modify([&](Mu& v){
                op.f(v, op.tree);
            });
        }
    }
}
void in::do_inits () {
    while (!init_ops.empty()) {
        auto inits = std::move(init_ops);
        for (auto& op : inits) {
            PushCurrentResource p (op.current_resource);
            op.item.modify([&](Mu& v){
                op.f(v);
            });
            do_swizzles();
        }
    }
}

void in::inner_from_tree (
    const DescriptionPrivate* desc, Mu& item, const Tree& tree,
    const Reference* unaddressable_ref, TempLocation* loc
) {
     // If description has a from_tree, just use that.
    if (auto from_tree = desc->from_tree()) {
        from_tree->f(item, tree);
        goto done;
    }
     // Now the behavior depends on what kind of tree we've been given
    switch (tree.form()) {
        case OBJECT: {
            if (desc->accepts_object()) {
                std::vector<Str> ks;
                for (auto& p : tree.data->as_known<Object>()) {
                    ks.emplace_back(p.first);
                }
                 // TODO: inner this too
                Reference ref (desc, &item);
                item_set_keys(ref, ks);
                for (auto& p : tree.data->as_known<Object>()) {
                    Reference attr = item_attr(ref, p.first);
                    auto attr_desc = DescriptionPrivate::get(attr.type());
                    auto attr_loc = KeyTempLocation(loc, p.first);
                    if (unaddressable_ref) {
                        auto attr_ur = item_attr(*unaddressable_ref, p.first);
                        attr.write([&](Mu& v){
                            inner_from_tree(
                                attr_desc, v, p.second,
                                &attr_ur, &attr_loc
                            );
                        });
                    }
                    else if (Mu* address = attr.address()) {
                        inner_from_tree(
                            attr_desc, *address, p.second,
                            null, &attr_loc
                        );
                    }
                    else {
                        attr.write([&](Mu& v){
                            inner_from_tree(
                                attr_desc, v, p.second,
                                &attr, &attr_loc
                            );
                        });
                    }
                }
                goto done;
            }
            else break;
        }
        case ARRAY: {
            if (desc->accepts_array()) {
                Reference ref (desc, &item);
                auto& a = tree.data->as_known<Array>();
                item_set_length(ref, a.size());
                for (usize i = 0; i < a.size(); i++) {
                    auto elem = item_elem(ref, i);
                    auto elem_desc = DescriptionPrivate::get(elem.type());
                    auto elem_loc = IndexTempLocation(loc, i);
                    if (unaddressable_ref) {
                        auto elem_ur = item_elem(*unaddressable_ref, i);
                        elem.write([&](Mu& v){
                            inner_from_tree(
                                elem_desc, v, a[i],
                                &elem_ur, &elem_loc
                            );
                        });
                    }
                    else if (Mu* address = elem.address()) {
                        inner_from_tree(
                            elem_desc, *address, a[i],
                            null, &elem_loc
                        );
                    }
                    else {
                        elem.write([&](Mu& v){
                            inner_from_tree(
                                elem_desc, v, a[i],
                                &elem, &elem_loc
                            );
                        });
                    }
                }
                goto done;
            }
            else break;
        }
        case ERROR: {
             // Dunno how we got this far but whatever
            std::rethrow_exception(tree.data->as_known<std::exception_ptr>());
        }
        default: {
             // All other tree types support the values descriptor
            if (auto values = desc->values()) {
                for (uint i = 0; i < values->n_values; i++) {
                    if (const Mu* v = values->value(i)->tree_to_value(tree)) {
                        values->assign(item, *v);
                        goto done;
                    }
                }
                break;
            }
            else break;
        }
    }
     // Nothing matched, so use delegate
    if (auto acr = desc->delegate_acr()) {
        auto del_desc = DescriptionPrivate::get(acr->type(&item));
        if (unaddressable_ref) {
            auto del_ur = unaddressable_ref->chain(acr);
            acr->write(item, [&](Mu& v){
                inner_from_tree(del_desc, v, tree, &del_ur, loc);
            });
        }
        else if (Mu* address = acr->address(item)) {
            inner_from_tree(del_desc, *address, tree, null, loc);
        }
        else {
            acr->write(item, [&](Mu& v){
                auto del_ur = Reference(&item, acr);
                inner_from_tree(del_desc, v, tree, &del_ur, loc);
            });
        }
        goto done;
    }
     // Still nothing?  Allow swizzle with no from_tree.
    if (desc->swizzle()) goto done;
     // If we got here, we failed to find any method to from_tree this item.
     // Go through maybe a little too much effort to figure out what went wrong
    if (tree.form() == OBJECT && (desc->values() || desc->accepts_array())) {
        throw X::InvalidForm(make_permanent(loc), tree);
    }
    else if (tree.form() == ARRAY && (desc->values() || desc->accepts_object())) {
        throw X::InvalidForm(make_permanent(loc), tree);
    }
    else if (desc->accepts_array() || desc->accepts_object()) {
        throw X::InvalidForm(make_permanent(loc), tree);
    }
    else if (desc->values()) {
        throw X::NoValueForName(make_permanent(loc), tree);
    }
    else {
        throw X::CannotFromTree(make_permanent(loc));
    }

    done:
     // Now register swizzle and init ops.  We're doing it now instead of before
     // to make sure that children get swizzled and initted before their parent.
    if (auto swizzle = desc->swizzle()) {
        swizzle_ops.emplace_back(
            swizzle->f,
            unaddressable_ref ? *unaddressable_ref : Reference(desc, &item),
            tree, current_resource()
        );
    }
    if (auto init = desc->init()) {
        init_ops.emplace_back(
            init->f,
            unaddressable_ref ? *unaddressable_ref : Reference(desc, &item),
            current_resource()
        );
    }
}

void item_from_tree (const Reference& item, const Tree& tree) {
    static bool in_from_tree = false;
    auto desc = DescriptionPrivate::get(item.type());
    auto loc = RootTempLocation(Resource());
    if (in_from_tree) {
        if (Mu* address = item.address()) {
            inner_from_tree(desc, *address, tree, null, &loc);
        }
        else {
            item.write([&](Mu& v){
                inner_from_tree(desc, v, tree, item, &loc);
            });
        }
    }
    else {
        if (!swizzle_ops.empty() || !init_ops.empty()) {
            AYU_INTERNAL_UGUU();
        }
        in_from_tree = true;
        try {
            if (Mu* address = item.address()) {
                inner_from_tree(desc, *address, tree, null, &loc);
            }
            else {
                item.write([&](Mu& v){
                    inner_from_tree(desc, v, tree, item, &loc);
                });
            }
            do_swizzles();
            do_inits();
            in_from_tree = false;
        }
        catch (...) {
            in_from_tree = false;
            swizzle_ops.clear();
            init_ops.clear();
            throw;
        }
    }
}

///// SHORTCUTS

String item_to_string (
    const Reference& item, PrintOptions opts
) {
    return tree_to_string(item_to_tree(item), opts);
}
void item_to_file (
    const Reference& item, Str filename, PrintOptions opts
) {
    return tree_to_file(item_to_tree(item), filename, opts);
}
void item_from_string (
    const Reference& item, Str src
) {
    return item_from_tree(item, tree_from_string(src));
}
void item_from_file (
    const Reference& item, Str filename
) {
    return item_from_tree(item, tree_from_file(filename));
}

///// ATTR OPERATIONS

void in::collect_key_str (StrVector& ks, Str k) {
     // This'll end up being N^2.  TODO: Test whether including an unordered_set
     // would speed this up (probably not).
    for (auto ksk : ks) if (k == ksk) return;
    ks.emplace_back(k);
}
void in::collect_key_string (StrVector& ks, String&& k) {
    for (auto ksk : ks) if (k == ksk) return;
    ks.owned_strings = std::make_unique<OwnedStringNode>(
        std::move(k), std::move(ks.owned_strings)
    );
    ks.emplace_back(ks.owned_strings->s);
}

void in::item_collect_keys (const Reference& item, StrVector& ks) {
    auto desc = DescriptionPrivate::get(item.type());
    if (auto acr = desc->keys_acr()) {
        Reference keys_ref = item.chain(acr);
         // Don't allow quick keys for non-addressable references, because they
         // could be created on the fly, which would invalidate their Strs when
         // .read() returns.  If we're calling get_keys on a non-adressable
         // reference, then we're already in a fairly worst-case performance
         // scenario, so deoptimizing a little won't change much.
        if (item.address()) {
             // Compare Type not std::type_info, since std::type_info can require a
             // string comparison.
            static Type type_vector_str = Type::CppType<std::vector<Str>>();
            static Type type_vector_string = Type::CppType<std::vector<String>>();
            if (keys_ref.type() == type_vector_str) {
                 // Optimize for std::vector<Str>
                keys_ref.read([&](const Mu& ksv){
                    for (auto& k : reinterpret_cast<const std::vector<Str>&>(ksv)) {
                        collect_key_str(ks, k);
                    }
                });
                return;
            }
            else if (keys_ref.type() == type_vector_string) {
                 // Capitulate to std::vector<String> too.
                keys_ref.read([&](const Mu& ksv){
                     // TODO: Flag accessor if it can be moved from?
                    for (auto& k : reinterpret_cast<const std::vector<String>&>(ksv)) {
                        collect_key_string(ks, String(k));
                    }
                });
                return;
            }
        }
         // General case, any type that serializes to an array of strings.
         // Do a read and then call item_to_tree on the result, because the
         // order of the keys might not be constant.  TODO: Move this check
         // into item_to_tree.
        keys_ref.read([&](const Mu& ksv){
            auto tree = item_to_tree(Reference(keys_ref.type(), &ksv));
            if (tree.form() != ARRAY) {
                throw X::InvalidKeysType(item, keys_ref.type());
            }
            for (Tree& e : tree.data->as_known<Array>()) {
                if (e.form() != STRING) {
                    throw X::InvalidKeysType(item, keys_ref.type());
                }
                collect_key_string(ks, std::move(e.data->as_known<String>()));
            }
        });
    }
    else if (auto attrs = desc->attrs()) {
         // TODO: Optimize for the case where there are no inherited attrs
        for (uint16 i = 0; i < attrs->n_attrs; i++) {
            auto attr = attrs->attr(i);
            auto acr = attr->acr();
            if (acr->attr_flags & ATTR_INHERIT) {
                item_collect_keys(item.chain(acr), ks);
            }
            else collect_key_str(ks, attr->key);
        }
    }
    else if (auto acr = desc->delegate_acr()) {
        return item_collect_keys(item.chain(acr), ks);
    }
    else throw X::NoAttrs(item);
}

void item_read_keys (
    const Reference& item,
    Callback<void(const std::vector<Str>&)> cb
) {
    StrVector ks;
    item_collect_keys(item, ks);
    cb(ks);
}
std::vector<String> item_get_keys (const Reference& item) {
    StrVector ks;
    item_collect_keys(item, ks);
    return std::vector<String>(ks.begin(), ks.end());
}

bool in::claim_key (std::vector<Str>& ks, Str k) {
     // This algorithm overall is O(N^3), we may be able to speed it up by
     // setting a flag if there are no inherited attrs, or maybe by using an
     // unordered_set?
    for (auto it = ks.begin(); it != ks.end(); it++) {
        if (*it == k) {
            ks.erase(it);
            return true;
        }
    }
    return false;
}

void in::item_claim_keys (
    const Reference& item,
    std::vector<Str>& ks,
    bool optional
) {
    static Type type_vector_str = Type::CppType<std::vector<Str>>();
    static Type type_vector_string = Type::CppType<std::vector<String>>();
    auto desc = DescriptionPrivate::get(item.type());
    if (auto acr = desc->keys_acr()) {
        if (!(acr->accessor_flags & ACR_READONLY)) {
             // Since we're passing null to acr->type(), we won't be able to use
             // quick keys for a dynamically typed acr, but I don't know why
             // you'd use reference_func() in keys() in the first place.
            Type keys_type = acr->type(null);
            if (keys_type == type_vector_str) {
                 // Note: don't use chain because it can include a modify op.
                item.write([&](Mu& v){
                    acr->write(v, [&](Mu& ksv){
                        reinterpret_cast<std::vector<Str>&>(ksv) = std::move(ks);
                        ks.clear(); // Unnecessary after move?
                    });
                });
            }
            else if (keys_type == type_vector_string) {
                item.write([&](Mu& v){
                    acr->write(v, [&](Mu& ksv){
                        reinterpret_cast<std::vector<String>&>(ksv) =
                            std::vector<String>(ks.begin(), ks.end());
                        ks.clear();
                    });
                });
            }
            else {
                 // General case.  This will probably be slow.
                Array a (ks.size());
                for (usize i = 0; i < ks.size(); i++) {
                    a[i] = Tree(ks[i]);
                }
                 // Manually follow the reference instead of using chain.  The
                 // reason for this is that if we call item_from_tree, it first
                 // calls item_set_length.  Since the keys ref is almost
                 // certainly unaddressable, this will cause an array of empty
                 // strings to be written to the object's keys.  In the case of
                 // std::unordered_map, this will give the map one element with
                 // a key of "".  Then when item_from_tree calls item_elem, it
                 // will throw ElemNotFound for all indexes > 0.
                 // TODO: Do this unwrapping in item_from_tree instead of here,
                 // to avoid similar problems in other scenarios.
                item.write([&](Mu& v){
                    acr->write(v, [&](Mu& ksv){
                        item_from_tree(
                            Reference(acr->type(&v), &ksv),
                            Tree(std::move(a))
                        );
                    });
                });
                ks.clear();
            }
        }
        else {
             // For readonly keys, get the keys and compare them.
            Reference keys_ref = item.chain(acr);
            Type keys_type = keys_ref.type();
            if (keys_type == type_vector_str) {
                 // Optimize for std::vector<Str>
                std::vector<Str> expected;
                keys_ref.read([&](const Mu& ksv){
                     // TODO: Flag accessors that can be moved from?
                    expected = reinterpret_cast<const std::vector<Str>&>(ksv);
                });
                for (auto& k : expected) {
                    if (claim_key(ks, k)) optional = false;
                    else if (!optional) throw X::MissingAttr(item, k);
                }
                return;
            }
            else if (keys_type == type_vector_string) {
                 // Capitulate for std::vector<String> too
                std::vector<String> expected;
                keys_ref.read([&](const Mu& ksv){
                    expected = reinterpret_cast<const std::vector<String>&>(ksv);
                });
                for (auto& k : expected) {
                    if (claim_key(ks, k)) optional = false;
                    else if (!optional) throw X::MissingAttr(item, k);
                }
                return;
            }
            else {
                 // General case.  This will probably be slow.
                auto tree = item_to_tree(keys_ref);
                if (tree.form() != ARRAY) {
                    throw X::InvalidKeysType(item, keys_ref.type());
                }
                for (auto& e : tree.data->as_known<Array>()) {
                    if (e.form() != STRING) {
                        throw X::InvalidKeysType(item, keys_ref.type());
                    }
                    Str k = e.data->as_known<String>();
                    if (claim_key(ks, k)) optional = false;
                    else if (!optional) throw X::MissingAttr(item, k);
                }
            }
            return;
        }
    }
    else if (auto attrs = desc->attrs()) {
         // Prioritize direct attrs
         // Note, we're using std::string as an optimization for bool[], it's
         // not actually text.
        std::string claimed_inherited (attrs->n_attrs, false);
        for (uint i = 0; i < attrs->n_attrs; i++) {
            auto attr = attrs->attr(i);
            auto acr = attr->acr();
            if (claim_key(ks, attr->key)) {
                 // If any attrs are given, all required attrs must be given
                 // (only matters if this item is an inherited attr)
                 // TODO: this should fail a test depending on the order of attrs
                optional = false;
                if (acr->attr_flags & ATTR_INHERIT) {
                    claimed_inherited[i] = true;
                }
            }
            else if (!(optional || (acr->attr_flags & (ATTR_OPTIONAL|ATTR_INHERIT)))) {
                throw X::MissingAttr(item, attr->key);
            }
        }
         // Then check inherited attrs
        for (uint i = 0; i < attrs->n_attrs; i++) {
            auto attr = attrs->attr(i);
            auto acr = attr->acr();
            if (acr->attr_flags & ATTR_INHERIT) {
                 // Skip if attribute was given directly, uncollapsed
                if (!claimed_inherited[i]) {
                    item_claim_keys(
                        item.chain(acr), ks,
                        optional || (acr->attr_flags & ATTR_OPTIONAL)
                    );
                }
            }
        }
    }
    else if (auto acr = desc->delegate_acr()) {
        return item_claim_keys(item.chain(acr), ks, optional);
    }
    else throw X::NoAttrs(item);
}

void item_set_keys (const Reference& item, const std::vector<Str>& ks) {
    std::vector<Str> claimed = ks;
    item_claim_keys(item, claimed, false);
    if (!claimed.empty()) {
        throw X::UnwantedAttr(item, claimed[0]);
    }
}

Reference item_maybe_attr (const Reference& item, Str key) {
    auto desc = DescriptionPrivate::get(item.type());
    if (desc->accepts_object()) {
        if (auto attrs = desc->attrs()) {
             // Note: This will likely be called once for each attr, making it
             // O(N^2) over the number of attrs.  If we want we could optimize for
             // large N by keeping a temporary map...somewhere
             //
             // First check direct attrs
            for (uint i = 0; i < attrs->n_attrs; i++) {
                auto attr = attrs->attr(i);
                auto acr = attr->acr();
                if (attr->key == key) {
                    return item.chain(acr);
                }
            }
             // Then inherited attrs
            for (uint i = 0; i < attrs->n_attrs; i++) {
                auto attr = attrs->attr(i);
                auto acr = attr->acr();
                if (acr->attr_flags & ATTR_INHERIT) {
                    Reference sub = item_maybe_attr(item.chain(acr), key);
                    if (sub) return sub;
                }
            }
        }
        if (auto attr_func = desc->attr_func()) {
            return item.chain_attr_func(attr_func->f, key);
        }
        else return Reference();
    }
    else if (auto acr = desc->delegate_acr()) {
        return item_maybe_attr(item.chain(acr), key);
    }
    else throw X::NoAttrs(item);
}
Reference item_attr (const Reference& item, Str key) {
    Reference r = item_maybe_attr(item, key);
    if (r) return r;
    else throw X::AttrNotFound(item, key);
}

///// ELEM OPERATIONS

usize item_get_length (const Reference& item) {
    auto desc = DescriptionPrivate::get(item.type());
    if (auto acr = desc->length_acr()) {
        usize l;
        item.read([&](const Mu& v){
            acr->read(v, [&](const Mu& lv){
                l = reinterpret_cast<const usize&>(lv);
            });
        });
        return l;
    }
    else if (auto elems = desc->elems()) {
        usize l = 0;
        for (usize i = 0; i < elems->n_elems; i++) {
            auto acr = elems->elem(i)->acr();
            if (acr->attr_flags & ATTR_INHERIT) {
                l += item_get_length(item.chain(acr));
            }
            else l += 1;
        }
        return l;
    }
    else if (auto acr = desc->delegate_acr()) {
        return item_get_length(item.chain(acr));
    }
    else throw X::NoElems(item);
}

void in::item_claim_length (const Reference& item, usize& claimed, usize len) {
    auto desc = DescriptionPrivate::get(item.type());
    if (auto acr = desc->length_acr()) {
        if (!(acr->accessor_flags & ACR_READONLY)) {
             // Note: don't use chain because it can include a modify op.
             // TODO: make this not necessary?
            usize remaining = (claimed <= len ? len - claimed : 0);
            item.write([&](Mu& v){
                acr->write(v, [&](Mu& lv){
                    reinterpret_cast<usize&>(lv) = remaining;
                });
            });
            claimed += remaining;
        }
        else {
             // For readonly length, just claim the returned length
            usize expected;
            item.chain(acr).read([&](const Mu& lv){
                expected = reinterpret_cast<const usize&>(lv);
            });
            claimed += expected;
        }
    }
    else if (auto elems = desc->elems()) {
        for (usize i = 0; i < elems->n_elems; i++) {
            auto acr = elems->elem(i)->acr();
            if (claimed >= len && acr->attr_flags & ATTR_OPTIONAL) {
                continue;
            }
            if (acr->attr_flags & ATTR_INHERIT) {
                item_claim_length(item.chain(acr), claimed, len);
            }
            else claimed += 1;
        }
    }
    else if (auto acr = desc->delegate_acr()) {
        item_claim_length(item.chain(acr), claimed, len);
    }
    else throw X::NoElems(item);
}

void item_set_length (const Reference& item, usize len) {
    usize claimed = 0;
    item_claim_length(item, claimed, len);
    if (claimed > len) {
        throw X::TooShort(item, claimed, len);
    }
    if (claimed < len) {
        throw X::TooLong(item, claimed, len);
    }
}

Reference item_maybe_elem (const Reference& item, usize index) {
    auto desc = DescriptionPrivate::get(item.type());
    if (desc->accepts_array()) {
        if (auto elems = desc->elems()) {
             // Including inheritance in this mix turns this from constant-time
             // to linear time, and thus serializing from linear time into
             // squared time.  We should probably have set a flag somewhere if
             // there no inherited elems so we can skip this loop.
            usize offset = 0;
            for (usize i = 0; i < elems->n_elems && offset <= index; i++) {
                auto acr = elems->elem(i)->acr();
                if (acr->attr_flags & ATTR_INHERIT) {
                    usize len = item_get_length(item.chain(acr));
                    if (index - offset < len) {
                        return item_maybe_elem(item.chain(acr), index - offset);
                    }
                    offset += len;
                }
                else {
                    if (offset == index) return item.chain(acr);
                    offset += 1;
                }
            }
            return Reference();
        }
        else if (auto elem_func = desc->elem_func()) {
            return item.chain_elem_func(elem_func->f, index);
        }
        else return Reference();
    }
    else if (auto acr = desc->delegate_acr()) {
        return item_maybe_elem(item.chain(acr), index);
    }
    else throw X::NoElems(item);
}
Reference item_elem (const Reference& item, usize index) {
    Reference r = item_maybe_elem(item, index);
    if (r) return r;
    else throw X::ElemNotFound(item, index);
}

///// REFERENCES AND LOCATIONS

Reference reference_from_location (Location loc) {
    if (!loc) return Reference();
    if (auto parent = loc.parent()) {
        if (auto key = loc.key()) {
            return reference_from_location(*parent).attr(*key);
        }
        else if (auto index = loc.index()) {
            return reference_from_location(*parent).elem(*index);
        }
        else AYU_INTERNAL_UGUU();
    }
    else if (auto res = loc.resource()) {
        return res->ref();
    }
    else AYU_INTERNAL_UGUU();
}

namespace in {
    static std::unordered_map<Reference, Location> location_cache;
    static usize keep_location_count = 0;
}

KeepLocationCache::KeepLocationCache () {
    keep_location_count++;
}
KeepLocationCache::~KeepLocationCache () {
    if (!--keep_location_count) {
        location_cache.clear();
    }
}

Location reference_to_location (const Reference& ref) {
    KeepLocationCache keep;
    if (location_cache.empty()) {
        recursive_scan_universe(
            [](const Reference& ref, Location loc){
                location_cache.emplace(ref, loc);
            }
        );
    }
    auto it = location_cache.find(ref);
    if (it != location_cache.end()) return it->second;
    else throw X::UnresolvedReference(ref);
}

String show_reference (const Reference& ref) {
    try {
        Location loc = reference_to_location(ref);
        return item_to_string(&loc);
    }
    catch (std::exception& e) {
        return cat("(An error occurred while showing this reference: "sv, e.what(), ')');
    }
}

void recursive_scan_universe (
    Callback<void(const Reference&, Location)> cb
) {
    for (auto& [_, resdat] : universe().resources) {
        recursive_scan_resource(Resource(&*resdat), cb);
    }
}

void recursive_scan_resource (
    Resource res,
    Callback<void(const Reference&, Location)> cb
) {
    if (res.state() == UNLOADED) return;
    recursive_scan(res.get_value(), Location(res), cb);
}

 // TODO: Skip atomic types T if AYU_DESCRIBE for T* has not been instantiated
void recursive_scan (
    const Reference& item, Location loc,
    Callback<void(const Reference&, Location)> cb
) {
    if (!item) return;
    cb(item, loc);

    auto desc = DescriptionPrivate::get(item.type());
    switch (desc->preference()) {
        case Description::PREFER_OBJECT: {
            for (auto& k : item_get_keys(item)) {
                recursive_scan(item_attr(item, k), Location(loc, k), cb);
            }
            return;
        }
        case Description::PREFER_ARRAY: {
            usize l = item_get_length(item);
            for (usize i = 0; i < l; i++) {
                recursive_scan(item_elem(item, i), Location(loc, i), cb);
            }
            return;
        }
        default: {
            if (auto acr = desc->delegate_acr()) {
                recursive_scan(item.chain(acr), loc, cb);
            }
            return;
        }
    }
}

///// DIAGNOSTIC HELP

///// ERRORS

namespace X {
    SerError::SerError (const Reference& item) {
        try {
            location = reference_to_location(item);
        }
        catch (std::exception& e) {
            location = make_error_location(std::current_exception());
        }
    }
}

} using namespace ayu;

AYU_DESCRIBE(ayu::X::SerError,
    elems(
        elem(base<X::Error>(), inherit)
    ),
    attrs(
        attr("ayu::X::Error", base<X::Error>(), inherit)
    )
)

AYU_DESCRIBE(ayu::X::CannotToTree,
    delegate(base<X::SerError>())
)
AYU_DESCRIBE(ayu::X::CannotFromTree,
    delegate(base<X::SerError>())
)
AYU_DESCRIBE(ayu::X::InvalidForm,
    delegate(base<X::SerError>())
)
AYU_DESCRIBE(ayu::X::NoNameForValue,
    delegate(base<X::SerError>())
)
AYU_DESCRIBE(ayu::X::NoValueForName,
    elems(
        elem(base<X::SerError>(), inherit),
        elem(&X::NoValueForName::tree)
    )
)
AYU_DESCRIBE(ayu::X::MissingAttr,
    elems(
        elem(base<X::SerError>(), inherit),
        elem(&X::MissingAttr::key)
    )
)
AYU_DESCRIBE(ayu::X::UnwantedAttr,
    elems(
        elem(base<X::SerError>(), inherit),
        elem(&X::UnwantedAttr::key)
    )
)
AYU_DESCRIBE(ayu::X::TooShort,
    attrs(
        attr("ayu::X::SerError", base<X::SerError>(), inherit),
        attr("min", &X::TooShort::min),
        attr("got", &X::TooShort::got)
    )
)
AYU_DESCRIBE(ayu::X::TooLong,
    attrs(
        attr("ayu::X::SerError", base<X::SerError>(), inherit),
        attr("max", &X::TooLong::max),
        attr("got", &X::TooLong::got)
    )
)
AYU_DESCRIBE(ayu::X::NoAttrs,
    delegate(base<X::SerError>())
)
AYU_DESCRIBE(ayu::X::NoElems,
    delegate(base<X::SerError>())
)
AYU_DESCRIBE(ayu::X::AttrNotFound,
    elems(
        elem(base<X::SerError>(), inherit),
        elem(&X::AttrNotFound::key)
    )
)
AYU_DESCRIBE(ayu::X::ElemNotFound,
    elems(
        elem(base<X::SerError>(), inherit),
        elem(&X::ElemNotFound::index)
    )
)
AYU_DESCRIBE(ayu::X::InvalidKeysType,
    elems(
        elem(base<X::SerError>(), inherit),
        elem(&X::InvalidKeysType::type)
    )
)
AYU_DESCRIBE(ayu::X::UnresolvedReference,
    delegate(base<X::SerError>()),
    elems(elem(&X::UnresolvedReference::type))
)

///// TESTS

#ifndef TAP_DISABLE_TESTS
#include <unordered_map>
#include "../../tap/tap.h"
#include "../describe-standard.h"

 // Putting these in a test namespace so their described names don't conflict
namespace ayu::test {
    struct ToTreeTest {
        int value;
    };

    enum ValuesTest {
        VTA,
        VTNULL,
        VTZERO,
        VTNAN
    };

    struct MemberTest {
        int a;
        int b;
         // Testing absence of copy constructor
        MemberTest (const MemberTest&) = delete;
         // C++20 doesn't let you aggregate-initialize classes with ANY
         // declared or deleted constructors any more.
        MemberTest (int a, int b) : a(a), b(b) { }
    };

    struct PrivateMemberTest {
        PrivateMemberTest (int s) : stuff(s) { }
      private:
        int stuff;
        AYU_FRIEND_DESCRIBE(PrivateMemberTest)
    };

    struct BaseTest : MemberTest {
        int c;
    };

    struct InheritTest : BaseTest {
        int d;
    };
    struct InheritOptionalTest : BaseTest {
        int d;
    };

    struct ElemTest {
        float x;
        float y;
        float z;
        void foo ();
    };

    struct ElemInheritTest {
        float a;
        ElemTest et1;
        float b;
        ElemTest et2;
        float c;
    };

    struct ElemsTest {
        std::vector<int> xs;
    };

     // Test usage of keys() with type std::vector<String>
    struct AttrsTest {
        std::unordered_map<String, int> xs;
    };
     // Test usage of keys() with type std::vector<Str>
    struct AttrsTest2 {
        std::unordered_map<String, int> xs;
    };

    struct DelegateTest {
        ElemTest et;
    };
    struct SwizzleTest {
        bool swizzled = false;
    };
    struct InitTest {
        int value;
        int value_after_init = 0;
    };
    struct NestedInitTest {
        InitTest it;
        int it_val = -1;
    };
    enum ScalarElemTest : uint8 {
    };
} using namespace ayu::test;

AYU_DESCRIBE(ayu::test::ToTreeTest,
    to_tree([](const ToTreeTest& x){ return Tree(x.value); }),
    from_tree([](ToTreeTest& x, const Tree& t){ x.value = int(t); })
)
const ValuesTest vtnan = VTNAN;
AYU_DESCRIBE(ayu::test::ValuesTest,
    values(
        value("vta", VTA),
        value(null, VTNULL),
        value(int(0), VTZERO),
        value_pointer(nan, &vtnan)
    )
)
AYU_DESCRIBE(ayu::test::MemberTest,
    attrs(
        attr("a", member(&MemberTest::a)),
        attr("b", &MemberTest::b)  // Implicit member()
    )
)
AYU_DESCRIBE(ayu::test::PrivateMemberTest,
    attrs(
        attr("stuff", &PrivateMemberTest::stuff)
    )
)

AYU_DESCRIBE(ayu::test::BaseTest,
    attrs(
        attr("MemberTest", base<MemberTest>()),
        attr("c", member(&BaseTest::c))
    )
)
AYU_DESCRIBE(ayu::test::InheritTest,
    attrs(
        attr("BaseTest", base<BaseTest>(), inherit),
        attr("d", &InheritTest::d)
    )
)
AYU_DESCRIBE(ayu::test::InheritOptionalTest,
    attrs(
        attr("BaseTest", base<BaseTest>(), inherit|optional),
        attr("d", &InheritOptionalTest::d)
    )
)
AYU_DESCRIBE(ayu::test::ElemTest,
    elems(
        elem(member(&ElemTest::x)),
        elem(&ElemTest::y),
        elem(member(&ElemTest::z))
    )
)
AYU_DESCRIBE(ayu::test::ElemInheritTest,
    elems(
        elem(&ElemInheritTest::a),
        elem(&ElemInheritTest::et1, inherit),
        elem(&ElemInheritTest::b, optional),
        elem(&ElemInheritTest::et2, inherit|optional),
        elem(&ElemInheritTest::c, optional)
    )
)
AYU_DESCRIBE(ayu::test::ElemsTest,
    length(value_funcs<usize>(
        [](const ElemsTest& v){
            return v.xs.size();
        },
        [](ElemsTest& v, usize l){
            v.xs.resize(l);
        }
    )),
    elem_func([](ElemsTest& v, usize i){
        return Reference(&v.xs.at(i));
    })
)
AYU_DESCRIBE(ayu::test::AttrsTest,
    keys(mixed_funcs<std::vector<String>>(
        [](const AttrsTest& v){
            std::vector<String> r;
            for (auto& p : v.xs) {
                r.emplace_back(p.first);
            }
            return r;
        },
        [](AttrsTest& v, const std::vector<String>& ks){
            v.xs.clear();
            for (auto& k : ks) {
                v.xs.emplace(k, 0);
            }
        }
    )),
    attr_func([](AttrsTest& v, Str k){
        return Reference(&v.xs.at(String(k)));
    })
)
AYU_DESCRIBE(ayu::test::AttrsTest2,
    keys(mixed_funcs<std::vector<Str>>(
        [](const AttrsTest2& v){
            std::vector<Str> r;
            for (auto& p : v.xs) {
                r.emplace_back(p.first);
            }
            return r;
        },
        [](AttrsTest2& v, const std::vector<Str>& ks){
            v.xs.clear();
            for (auto& k : ks) {
                v.xs.emplace(k, 0);
            }
        }
    )),
    attr_func([](AttrsTest2& v, Str k){
        return Reference(&v.xs.at(String(k)));
    })
)
AYU_DESCRIBE(ayu::test::DelegateTest,
    delegate(member(&DelegateTest::et))
)
AYU_DESCRIBE(ayu::test::SwizzleTest,
    swizzle([](SwizzleTest& v, const Tree&){
        v.swizzled = true;
    })
)
AYU_DESCRIBE(ayu::test::InitTest,
    delegate(member(&InitTest::value)),
    init([](InitTest& v){
        v.value_after_init = v.value + 1;
    })
)
AYU_DESCRIBE(ayu::test::NestedInitTest,
    attrs(attr("it", &NestedInitTest::it)),
    init([](NestedInitTest& v){
        v.it_val = v.it.value_after_init;
    })
)
AYU_DESCRIBE(ayu::test::ScalarElemTest,
    elems(
        elem(value_funcs<uint8>(
            [](const ScalarElemTest& v) -> uint8 {
                return uint8(v) >> 4;
            },
            [](ScalarElemTest& v, uint8 m){
                v = ScalarElemTest((uint8(v) & 0xf) | (m << 4));
            }
        )),
        elem(value_funcs<uint8>(
            [](const ScalarElemTest& v) -> uint8 {
                return uint8(v) & 0xf;
            },
            [](ScalarElemTest& v, uint8 m){
                v = ScalarElemTest((uint8(v) & 0xf0) | (m & 0xf));
            }
        ))
    )
)

static tap::TestSet tests ("base/ayu/serialize", []{
    using namespace tap;
    ok(get_description_by_type_info(typeid(MemberTest)), "Description was registered");

    auto ttt = ToTreeTest{5};
    Tree tttt = item_to_tree(&ttt);
    is(tttt, Tree(5), "item_to_tree works with to_tree descriptor");

    ValuesTest vtt = VTA;
    is(item_to_tree(&vtt), tree_from_string("\"vta\""), "item_to_tree works with string value");
    vtt = VTNULL;
    is(item_to_tree(&vtt), tree_from_string("null"), "item_to_tree works with null value");
    vtt = VTZERO;
    is(item_to_tree(&vtt), tree_from_string("0"), "item_to_tree works with int value");
    vtt = VTNAN;
    is(item_to_tree(&vtt), tree_from_string("+nan"), "item_to_tree works with double value");
    vtt = ValuesTest(999);
    doesnt_throw([&]{ item_from_string(&vtt, "\"vta\""); });
    is(vtt, VTA, "item_from_tree works with string value");
    doesnt_throw([&]{ item_from_string(&vtt, "null"); });
    is(vtt, VTNULL, "item_from_tree works with string value");
    doesnt_throw([&]{ item_from_string(&vtt, "0"); });
    is(vtt, VTZERO, "item_from_tree works with string value");
    doesnt_throw([&]{ item_from_string(&vtt, "+nan"); });
    is(vtt, VTNAN, "item_from_tree works with string value");

    auto mt = MemberTest(3, 4);
    Tree mtt = item_to_tree(&mt);
    is(mtt, tree_from_string("{a:3 b:4}"), "item_to_tree works with attrs descriptor");

    auto pmt = PrivateMemberTest(4);
    Tree pmtt = item_to_tree(&pmt);
    is(pmtt, tree_from_string("{stuff:4}"), "AYU_FRIEND_DESCRIBE works");

    item_from_string(&mt, "{a:87 b:11}");
    is(mt.a, 87, "item_from_tree works with attrs descriptor (a)");
    is(mt.b, 11, "item_from_tree works with attrs descriptor (b)");
    item_from_string(&mt, "{b:92 a:47}");
    is(mt.a, 47, "item_from_tree works with attrs out of order (a)");
    is(mt.b, 92, "item_from_tree works with attrs out of order (b)");
    throws<X::MissingAttr>([&]{
        item_from_string(&mt, "{a:16}");
    }, "item_from_tree throws on missing attr with attrs descriptor");
    throws<X::WrongForm>([&]{
        item_from_string(&mt, "{a:41 b:foo}");
    }, "item_from_tree throws X::WrongForm when attr has wrong form");
    throws<X::CantRepresent>([&]{
        item_from_string(&mt, "{a:41 b:4.3}");
    }, "item_from_tree throws X::CantRepresent when int attr isn't integer");
    throws<X::InvalidForm>([&]{
        item_from_string(&mt, "[54 43]");
    }, "item_from_tree throws X::InvalidForm when trying to make attrs object from array");
    throws<X::UnwantedAttr>([&]{
        item_from_string(&mt, "{a:0 b:1 c:60}");
    }, "item_from_tree throws on extra attr");

    auto bt = BaseTest{{-1, -2}, -3};
    Tree btt = item_to_tree(&bt);
    is(btt, tree_from_string("{MemberTest:{a:-1,b:-2} c:-3}"), "item_to_tree with base attr");
    Tree from_tree_bt1 = tree_from_string("{c:-4,MemberTest:{a:-5,b:-6}}");
    item_from_tree(&bt, from_tree_bt1);
    is(bt.b, -6, "item_from_tree with base attr");
    throws<X::MissingAttr>([&]{
        item_from_string(&bt, "{a:-7,b:-8,c:-9}");
    }, "item_from_tree with base attr throws when collapsed but inherit is not specified");

    auto it = InheritTest{{{99, 88}, 77}, 66};
    Tree itt = item_to_tree(&it);
    is(itt, tree_from_string("{MemberTest:{a:99,b:88} c:77 d:66}"), "Inherit works with item_to_tree");
    Tree from_tree_it1 = tree_from_string("{d:55 c:44 MemberTest:{a:33 b:22}}");
    item_from_tree(&it, from_tree_it1);
    is(it.a, 33, "Inherit works with item_from_tree");
    Tree from_tree_it2 = tree_from_string("{d:51 BaseTest:{c:41 MemberTest:{b:31 a:21}}}");
    item_from_tree(&it, from_tree_it2);
    is(it.b, 31, "Inherit works when not collapsed");

    auto iot = InheritOptionalTest{{{23, 24}, 25}, 26};
    Tree from_tree_iot1 = tree_from_string("{d:44}");
    item_from_tree(&iot, from_tree_iot1);
    is(iot.d, 44, "Inherit optional works");
    is(iot.a, 23, "Didn't set attrs of optional inherited attrs");
    throws<X::MissingAttr>([&]{
        item_from_tree(&iot, tree_from_string("{d:34 MemberTest:{a:56 b:67}}"));
    }, "Optional inherited attrs need either all or no attrs");
    todo(1);
    throws<X::MissingAttr>([&]{
        item_from_tree(&iot, tree_from_string("{d:34 c:78}"));
    }, "Optional inherited attrs need either all or no attrs (2)");

    auto et = ElemTest{0.5, 1.5, 2.5};
    Tree ett = item_to_tree(&et);
    is(ett, tree_from_string("[0.5 1.5 2.5]"), "item_to_tree with elems descriptor");
    Tree from_tree_et1 = tree_from_string("[3.5 4.5 5.5]");
    item_from_tree(&et, from_tree_et1);
    is(et.y, 4.5, "item_from_tree with elems descriptor");
    throws<X::TooShort>([&]{
        item_from_string(&et, "[6.5 7.5]");
    }, "item_from_tree throws on too short array with elems descriptor");
    throws<X::TooLong>([&]{
        item_from_string(&et, "[6.5 7.5 8.5 9.5]");
    }, "item_from_tree throws on too long array with elems descriptor");
    throws<X::InvalidForm>([&]{
        item_from_string(&et, "{x:1.1 y:2.2}");
    }, "item_from_tree throws X::InvalidForm when trying to make elems thing from object");

    auto eit = ElemInheritTest{0xa, {1, 2, 3}, 0xb, {4, 5, 6}, 0xc};
    is(*(float*)item_elem(&eit, 7), 6, "item_elem with inherited elems");
    is(*(float*)item_elem(&eit, 8), 0xc, "item_elem with inherited elems");
    ok(!item_maybe_elem(&eit, 9).type(), "item_elem out of range with inherited elems");
    Tree eitt = item_to_tree(&eit);
    is(eitt, tree_from_string("[0xa 1 2 3 0xb 4 5 6 0xc]"), "item_to_tree with inherited elems");
    Tree from_tree_eit1 = tree_from_string("[0xaa 11 22 33 0xbb 44 55 66 0xcc]");
    item_from_tree(&eit, from_tree_eit1);
    is(eit.c, 0xcc, "item_from_tree with inherited elems");
    is(eit.et2.y, 55, "item_from_tree with inherited elems (inherited elem)");
    Tree from_tree_eit2 = tree_from_string("[0xaaa 111 222 333]");
    item_from_tree(&eit, from_tree_eit2);
    is(eit.a, 0xaaa, "item_from_tree with optional elems (provided)");
    is(eit.b, 0xbb, "item_from_tree with optional elems (unprovided)");
    throws<X::TooShort>([&]{
        item_from_string(&eit, "[1 2 3]");
    }, "item_from_tree throws on too short array with inherited elems");
    throws<X::TooLong>([&]{
        item_from_string(&eit, "[0xa 1 2 3 0xb 4 5 6 0xc 0xd]");
    }, "item_from_tree throws on too long array with inherited elems");
    throws<X::TooShort>([&]{
        item_from_string(&eit, "[0xa 1 2 3 0xb 4]");
    }, "item_from_tree throws on partially provided optional inherited element");

    auto est = ElemsTest{{1, 3, 6, 10, 15, 21}};
    is(item_get_length(&est), 6u, "item_get_length");
    int answer = 0;
    doesnt_throw([&]{
        item_elem(&est, 5).read_as<int>([&](const int& v){ answer = v; });
    }, "item_elem and Reference::read_as");
    is(answer, 21, "item_elem gives correct answer");
    throws<std::out_of_range>([&]{
        item_elem(&est, 6);
    }, "item_elem can throw on out of bounds index (from user-defined function)");
    item_set_length(&est, 5);
    is(est.xs.size(), 5u, "item_set_length shrink");
    throws<std::out_of_range>([&]{
        item_elem(&est, 5);
    }, "item_elem reflects new length");
    item_set_length(&est, 9);
    is(est.xs.size(), 9u, "item_set_length grow");
    doesnt_throw([&]{
        item_elem(&est, 8).write_as<int>([](int& v){ v = 99; });
    }, "item_elem and Reference::write_as");
    is(est.xs.at(8), 99, "writing to elem works");
    is(item_to_tree(&est), tree_from_string("[1 3 6 10 15 0 0 0 99]"), "item_to_tree with length and elem_func");
    doesnt_throw([&]{
        item_from_string(&est, "[5 2 0 4]");
    }, "item_from_tree with length and elem_func doesn't throw");
    is(est.xs.at(3), 4, "item_from_tree works with elem_func");

    auto ast = AttrsTest{{{"a", 11}, {"b", 22}}};
    std::vector<String> keys_string = item_get_keys(&ast);
    std::vector<Str> keys (keys_string.begin(), keys_string.end());
    is(keys.size(), 2u, "item_get_keys (size)");
    ok((keys[0] == "a" && keys[1] == "b") || (keys[0] == "b" && keys[1] == "a"),
        "item_get_keys (contents)"
    );
    answer = 0;
    doesnt_throw([&]{
        item_attr(&ast, "b").read_as<int>([&](const int& v){ answer = v; });
    }, "item_attr and Reference::read_as");
    is(answer, 22, "item_attr gives correct answer");
    throws<std::out_of_range>([&]{
        item_attr(&ast, "c");
    }, "item_attr can throw on missing key (from user-defined function)");
    keys = std::vector<Str>{"c", "d"};
    item_set_keys(&ast, keys);
    is(ast.xs.find("a"), ast.xs.end(), "item_set_keys removed key");
    is(ast.xs.at("c"), 0, "item_set_keys added key");
    doesnt_throw([&]{
        item_attr(&ast, "d").write_as<int>([](int& v){ v = 999; });
    }, "item_attr and Reference::write_as");
    is(ast.xs.at("d"), 999, "writing to attr works");
    is(item_to_tree(&ast), tree_from_string("{c:0,d:999}"), "item_to_tree with keys and attr_func");
    doesnt_throw([&]{
        item_from_string(&ast, "{e:88,f:34}");
    }, "item_from_tree with keys and attr_func doesn't throw");
    is(ast.xs.at("f"), 34, "item_from_tree works with attr_func");

    auto ast2 = AttrsTest2{{{"a", 11}, {"b", 22}}};
    item_read_keys(&ast2, [&](const std::vector<Str>& ks){
        is(ks.size(), 2u, "item_read_keys (size)");
        ok((ks[0] == "a" && ks[1] == "b") || (ks[0] == "b" && ks[1] == "a"),
            "item_read_keys (contents)"
        );
    });
    answer = 0;
    doesnt_throw([&]{
        item_attr(&ast2, "b").read_as<int>([&](const int& v){ answer = v; });
    }, "item_attr and Reference::read_as");
    is(answer, 22, "item_attr gives correct answer");
    throws<std::out_of_range>([&]{
        item_attr(&ast2, "c");
    }, "item_attr can throw on missing key (from user-defined function)");
    keys = std::vector<Str>{"c", "d"};
    item_set_keys(&ast2, keys);
    is(ast2.xs.find("a"), ast2.xs.end(), "item_set_keys removed key");
    is(ast2.xs.at("c"), 0, "item_set_keys added key");
    doesnt_throw([&]{
        item_attr(&ast2, "d").write_as<int>([](int& v){ v = 999; });
    }, "item_attr and Reference::write_as");
    is(ast2.xs.at("d"), 999, "writing to attr works");
    is(item_to_tree(&ast2), tree_from_string("{c:0,d:999}"), "item_to_tree with keys and attr_func");
    doesnt_throw([&]{
        item_from_string(&ast2, "{e:88,f:34}");
    }, "item_from_tree with keys and attr_func doesn't throw");
    is(ast2.xs.at("f"), 34, "item_from_tree works with attr_func");

    auto dt = DelegateTest{{4, 5, 6}};
    is(item_to_tree(&dt), tree_from_string("[4 5 6]"), "item_to_tree with delegate");
    doesnt_throw([&]{
        item_from_string(&dt, "[7 8 9]");
    });
    is(dt.et.y, 8, "item_from_tree with delegate");
    is(item_elem(&dt, 2).address_as<float>(), &dt.et.z, "item_elem works with delegate");

    std::vector<ToTreeTest> tttv {{444}, {333}};
    is(item_to_tree(&tttv), tree_from_string("[444 333]"), "template describe on std::vector works");
    doesnt_throw([&]{
        item_from_string(&tttv, "[222 111 666 555]");
    });
    is(tttv[3].value, 555, "from_tree works with template describe on std::vector");

    std::vector<SwizzleTest> stv;
    doesnt_throw([&]{
        item_from_string(&stv, "[{}{}{}{}{}{}]");
    });
    ok(stv.at(4).swizzled, "Basic swizzle works");

    InitTest initt {4};
    doesnt_throw([&]{
        item_from_string(&initt, "6");
    });
    is(initt.value_after_init, 7, "Basic init works");
    NestedInitTest nit {{3}};
    doesnt_throw([&]{
        item_from_string(&nit, "{it:55}");
    });
    is(nit.it_val, 56, "Children get init() before parent");

    ScalarElemTest set = ScalarElemTest(0xab);
    is(item_to_tree(&set), tree_from_string("[0xa 0xb]"), "Can use elems() on scalar type (to_tree)");
    doesnt_throw([&]{
        item_from_string(&set, "[0xc 0xd]");
    });
    is(set, ScalarElemTest(0xcd), "Can use elems() on scalar type (from_tree)");

    done_testing();
});
#endif
