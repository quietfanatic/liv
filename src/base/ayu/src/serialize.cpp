#include "serialize-private.h"

#include <cassert>
#include "../describe.h"
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

Tree in::ser_to_tree (const Traversal& trav) {
    try {
        if (auto to_tree = trav.desc->to_tree()) {
            return to_tree->f(*trav.item);
        }
        if (auto values = trav.desc->values()) {
            for (uint i = 0; i < values->n_values; i++) {
                Tree r = values->value(i)->value_to_tree(values, *trav.item);
                if (r.has_value()) return r;
            }
        }
        switch (trav.desc->preference()) {
            case Description::PREFER_OBJECT: {
                Object o;
                StrVector ks;
                ser_collect_keys(trav, ks);
                for (auto& k : ks) {
                    ser_attr(
                        trav, k, ACR_READ, [&](const Traversal& child)
                    {
                         // Don't serialize readonly attributes, because they
                         // can't be deserialized.
                        if (!child.readonly) {
                            o.emplace_back(k, ser_to_tree(child));
                        }
                    });
                }
                return Tree(std::move(o));
            }
            case Description::PREFER_ARRAY: {
                Array a;
                usize l = ser_get_length(trav);
                for (usize i = 0; i < l; i++) {
                    ser_elem(
                        trav, i, ACR_READ, [&](const Traversal& child)
                    {
                         // Readonly elems are problematic, because they can't
                         // just be skipped without changing the order of other
                         // elems.  We should probably just forbid them.
                         // TODO: do that.
                        a.emplace_back(ser_to_tree(child));
                    });
                }
                return Tree(std::move(a));
            }
            default: {
                if (auto acr = trav.desc->delegate_acr()) {
                    Tree r;
                    trav_delegate(
                        trav, acr, ACR_READ, [&](const Traversal& child)
                    {
                        r = ser_to_tree(child);
                    });
                    return r;
                }
                else if (trav.desc->values()) {
                    throw X::NoNameForValue(trav_location(trav));
                }
                else throw X::CannotToTree(trav_location(trav));
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
 // TODO: Add location parameter
Tree item_to_tree (const Reference& item) {
    Tree r;
    trav_start(
        item, Location(Resource()), ACR_READ, [&](const Traversal& trav)
    {
        r = ser_to_tree(trav);
    });
    return r;
}

///// FROM_TREE
void in::ser_from_tree (const Traversal& trav, const Tree& tree) {
     // If description has a from_tree, just use that.
    if (auto from_tree = trav.desc->from_tree()) {
        from_tree->f(*trav.item, tree);
        goto done;
    }
     // Now the behavior depends on what kind of tree we've been given
    switch (tree.form()) {
        case OBJECT: {
            if (trav.desc->accepts_object()) {
                auto& o = tree.data->as_known<Object>();
                std::vector<Str> ks;
                for (auto& p : o) {
                    ks.emplace_back(p.first);
                }
                ser_set_keys(trav, std::move(ks));
                for (auto& p : o) {
                    ser_attr(
                        trav, p.first, ACR_WRITE, [&](const Traversal& child)
                    {
                        ser_from_tree(child, p.second);
                    });
                }
                goto done;
            }
            else break;
        }
        case ARRAY: {
            if (trav.desc->accepts_array()) {
                auto& a = tree.data->as_known<Array>();
                ser_set_length(trav, a.size());
                for (usize i = 0; i < a.size(); i++) {
                    ser_elem(trav, i, ACR_WRITE, [&](const Traversal& child){
                        ser_from_tree(child, a[i]);
                    });
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
            if (auto values = trav.desc->values()) {
                for (uint i = 0; i < values->n_values; i++) {
                    if (const Mu* v = values->value(i)->tree_to_value(tree)) {
                        values->assign(*trav.item, *v);
                        goto done;
                    }
                }
                break;
            }
            else break;
        }
    }
     // Nothing matched, so use delegate
    if (auto acr = trav.desc->delegate_acr()) {
        trav_delegate(
            trav, acr, ACR_WRITE, [&](const Traversal& child)
        {
            ser_from_tree(child, tree);
        });
        goto done;
    }
     // Still nothing?  Allow swizzle with no from_tree.
    if (trav.desc->swizzle()) goto done;
     // If we got here, we failed to find any method to from_tree this item.
     // Go through maybe a little too much effort to figure out what went wrong
    if (tree.form() == OBJECT &&
        (trav.desc->values() || trav.desc->accepts_array())
    ) {
        throw X::InvalidForm(trav_location(trav), tree);
    }
    else if (tree.form() == ARRAY &&
        (trav.desc->values() || trav.desc->accepts_object())
    ) {
        throw X::InvalidForm(trav_location(trav), tree);
    }
    else if (trav.desc->accepts_array() || trav.desc->accepts_object()) {
        throw X::InvalidForm(trav_location(trav), tree);
    }
    else if (trav.desc->values()) {
        throw X::NoValueForName(trav_location(trav), tree);
    }
    else {
        throw X::CannotFromTree(trav_location(trav));
    }

    done:
     // Now register swizzle and init ops.  We're doing it now instead of at the
     // beginning to make sure that children get swizzled and initted before
     // their parent.
    auto swizzle = trav.desc->swizzle();
    auto init = trav.desc->init();
    if (swizzle || init) {
        Reference ref = trav_reference(trav);
        if (swizzle) {
            swizzle_ops.emplace_back(
                swizzle->f, ref, tree, current_resource()
            );
        }
        if (init) {
            init_ops.emplace_back(
                init->f, ref, current_resource()
            );
        }
    }
}

void in::ser_do_swizzles () {
     // Swizzling might add more swizzle ops.  It'd be weird, but it's possible
     // and we should support it.
    while (!swizzle_ops.empty()) {
         // Explicitly assign to clear swizzle_ops
        auto swizzles = std::move(swizzle_ops);
        for (auto& op : swizzles) {
            PushCurrentResource p (op.current_resource);
            op.item.modify([&](Mu& v){
                op.f(v, op.tree);
            });
        }
    }
}
void in::ser_do_inits () {
     // Initting might add some more init ops.
    while (!init_ops.empty()) {
        auto inits = std::move(init_ops);
        for (auto& op : inits) {
            PushCurrentResource p (op.current_resource);
            op.item.modify([&](Mu& v){
                op.f(v);
            });
             // Initting might even add more swizzle ops.
            ser_do_swizzles();
        }
    }
}

void item_from_tree (const Reference& item, const Tree& tree) {
    static bool in_from_tree = false;
    if (in_from_tree) {
         // If we're reentering, swizzles and inits will be done later.
        trav_start(
            item, Location(Resource()), ACR_WRITE, [&](const Traversal& trav)
        {
            ser_from_tree(trav, tree);
        });
    }
    else {
        if (!swizzle_ops.empty() || !init_ops.empty()) {
            AYU_INTERNAL_UGUU();
        }
        in_from_tree = true;
        try {
            trav_start(
                item, Location(Resource()), ACR_WRITE, [&](const Traversal& trav)
            {
                ser_from_tree(trav, tree);
            });
            ser_do_swizzles();
            ser_do_inits();
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

///// ATTR OPERATIONS
void in::ser_collect_key_str (StrVector& ks, Str k) {
     // This'll end up being N^2.  TODO: Test whether including an unordered_set
     // would speed this up (probably not).
    for (auto ksk : ks) if (k == ksk) return;
    ks.emplace_back(k);
}
void in::ser_collect_key_string (StrVector& ks, String&& k) {
    for (auto ksk : ks) if (k == ksk) return;
    ks.owned_strings = std::make_unique<OwnedStringNode>(
        std::move(k), std::move(ks.owned_strings)
    );
    ks.emplace_back(ks.owned_strings->s);
}

void in::ser_collect_keys (const Traversal& trav, StrVector& ks) {
    if (auto acr = trav.desc->keys_acr()) {
        Type keys_type = acr->type(trav.item);
         // Compare Type not std::type_info, since std::type_info can require a
         // string comparison.
        static Type type_vector_str = Type::CppType<std::vector<Str>>();
        static Type type_vector_string = Type::CppType<std::vector<String>>();
        if (keys_type == type_vector_str) {
             // Optimize for std::vector<Str>
            acr->read(*trav.item, [&](const Mu& ksv){
                auto& str_ksv = reinterpret_cast<const std::vector<Str>&>(ksv);
                 // If this item is not addressable, it may be dynamically
                 // generated, which means the Strs may go out of scope before
                 // we get a chance to use them, so copy them.
                if (trav.addressable) {
                    for (auto& k : str_ksv) {
                        ser_collect_key_str(ks, k);
                    }
                }
                else {
                    for (auto& k : str_ksv) {
                        ser_collect_key_string(ks, String(k));
                    }
                }
            });
        }
        else if (keys_type == type_vector_string) {
             // Capitulate to std::vector<String> too.
            acr->read(*trav.item, [&](const Mu& ksv){
                 // TODO: Flag accessor if it can be moved from?
                for (auto& k : reinterpret_cast<const std::vector<String>&>(ksv)) {
                    ser_collect_key_string(ks, String(k));
                }
            });
        }
        else {
             // General case, any type that serializes to an array of strings.
            acr->read(*trav.item, [&](const Mu& ksv){
                 // We might be able to optimize this more, but it's not that
                 // important.
                auto tree = item_to_tree(Reference(keys_type, &ksv));
                if (tree.form() != ARRAY) {
                    throw X::InvalidKeysType(trav_location(trav), keys_type);
                }
                for (Tree& e : tree.data->as_known<Array>()) {
                    if (e.form() != STRING) {
                        throw X::InvalidKeysType(trav_location(trav), keys_type);
                    }
                    ser_collect_key_string(
                        ks, std::move(e.data->as_known<String>())
                    );
                }
            });
        }
    }
    else if (auto attrs = trav.desc->attrs()) {
         // TODO: Optimize for the case where there are no inherited attrs
        for (uint16 i = 0; i < attrs->n_attrs; i++) {
            auto attr = attrs->attr(i);
            auto acr = attr->acr();
             // TODO: Parents inheriting children is weird so let's rename
             // the inherit flag
            if (acr->attr_flags & ATTR_INHERIT) {
                trav_attr(
                    trav, acr, attr->key, ACR_READ, [&](const Traversal& child)
                {
                    ser_collect_keys(child, ks);
                });
            }
            else ser_collect_key_str(ks, attr->key);
        }
    }
    else if (auto acr = trav.desc->delegate_acr()) {
        trav_delegate(trav, acr, ACR_READ, [&](const Traversal& child){
            ser_collect_keys(child, ks);
        });
    }
    else throw X::NoAttrs(trav_location(trav));
}

void item_read_keys (
    const Reference& item,
    Callback<void(const std::vector<Str>&)> cb
) {
    StrVector ks;
    trav_start(
        item, Location(Resource()), ACR_READ, [&](const Traversal& trav)
    {
        ser_collect_keys(trav, ks);
    });
    cb(ks);
}
std::vector<String> item_get_keys (const Reference& item) {
    StrVector ks;
    trav_start(
        item, Location(Resource()), ACR_READ, [&](const Traversal& trav)
    {
        ser_collect_keys(trav, ks);
    });
    return std::vector<String>(ks.begin(), ks.end());
}

bool in::ser_claim_key (std::vector<Str>& ks, Str k) {
     // This algorithm overall is O(N^3), we may be able to speed it up by
     // setting a flag if there are no inherited attrs, or maybe by using an
     // unordered_set?
     // TODO: Just use a bool array for claiming instead of erasing from
     // the vector.
    for (auto it = ks.begin(); it != ks.end(); it++) {
        if (*it == k) {
            ks.erase(it);
            return true;
        }
    }
    return false;
}

void in::ser_claim_keys (
    const Traversal& trav,
    std::vector<Str>& ks,
    bool optional
) {
    if (auto acr = trav.desc->keys_acr()) {
        Type keys_type = acr->type(trav.item);
        if (!(acr->accessor_flags & ACR_READONLY)) {
            static Type type_vector_str = Type::CppType<std::vector<Str>>();
            static Type type_vector_string = Type::CppType<std::vector<String>>();
            if (keys_type == type_vector_str) {
                 // Optimize for std::vector<Str>
                acr->write(*trav.item, [&](Mu& ksv){
                    reinterpret_cast<std::vector<Str>&>(ksv) = std::move(ks);
                });
            }
            else if (keys_type == type_vector_string) {
                 // Compromise for std::vector<String> too
                acr->write(*trav.item, [&](Mu& ksv){
                    reinterpret_cast<std::vector<String>&>(ksv) =
                        std::vector<String>(ks.begin(), ks.end());
                });
            }
            else {
                 // General case: call item_from_tree on the keys.  This will
                 // be slow.
                Array a (ks.size());
                for (usize i = 0; i < ks.size(); i++) {
                    a[i] = Tree(ks[i]);
                }
                acr->write(*trav.item, [&](Mu& ksv){
                    item_from_tree(
                        Reference(keys_type, &ksv), Tree(std::move(a))
                    );
                });
            }
            ks.clear();
        }
        else {
             // For readonly keys, get the keys and compare them.
             // TODO: This can probably be optimized more
            StrVector ks;
            ser_collect_keys(trav, ks);
            for (auto k : ks) {
                if (ser_claim_key(ks, k)) {
                    optional = false;
                }
                else if (!optional) {
                    throw X::MissingAttr(trav_location(trav), k);
                }
            }
            return;
        }
    }
    else if (auto attrs = trav.desc->attrs()) {
         // Prioritize direct attrs
         // I don't think it's possible for n_attrs to be large enough to
         // overflow the trav...right?  The max description size is 64K and an
         // attr always consumes at least 14 bytes, so the max n_attrs is
         // something like 4500.  TODO: enforce a reasonable max n_attrs in
         // descriptors-internal.h.
        bool claimed_inherited [attrs->n_attrs];
        for (uint i = 0; i < attrs->n_attrs; i++) {
            auto attr = attrs->attr(i);
            auto acr = attr->acr();
            if (ser_claim_key(ks, attr->key)) {
                 // If any attrs are given, all required attrs must be given
                 // (only matters if this item is an inherited attr)
                 // TODO: this should fail a test depending on the order of attrs
                optional = false;
                if (acr->attr_flags & ATTR_INHERIT) {
                    claimed_inherited[i] = true;
                }
            }
            else if (optional || acr->attr_flags & (ATTR_OPTIONAL|ATTR_INHERIT)) {
                 // Allow omitting optional or inherited attrs
            }
            else {
                throw X::MissingAttr(trav_location(trav), attr->key);
            }
        }
         // Then check inherited attrs
        for (uint i = 0; i < attrs->n_attrs; i++) {
            auto attr = attrs->attr(i);
            auto acr = attr->acr();
            if (acr->attr_flags & ATTR_INHERIT) {
                 // Skip if attribute was given directly, uncollapsed
                if (!claimed_inherited[i]) {
                    trav_attr(
                        trav, acr, attr->key, ACR_WRITE,
                        [&](const Traversal& child)
                    {
                        ser_claim_keys(
                            child, ks,
                            optional || acr->attr_flags & ATTR_OPTIONAL
                        );
                    });
                }
            }
        }
    }
    else if (auto acr = trav.desc->delegate_acr()) {
        trav_delegate(trav, acr, ACR_WRITE, [&](const Traversal& child){
            ser_claim_keys(child, ks, optional);
        });
    }
    else throw X::NoAttrs(trav_location(trav));
}

void in::ser_set_keys (const Traversal& trav, std::vector<Str>&& ks) {
    ser_claim_keys(trav, ks, false);
    if (!ks.empty()) {
        throw X::UnwantedAttr(trav_location(trav), ks[0]);
    }
}

void item_set_keys (const Reference& item, const std::vector<Str>& ks) {
    trav_start(
        item, Location(Resource()), ACR_WRITE, [&](const Traversal& trav)
    {
        auto ks_copy = ks;
        ser_set_keys(trav, std::move(ks_copy));
    });
}

bool in::ser_maybe_attr (
    const Traversal& trav, Str key, AccessOp op, TravCallback cb
) {
    if (auto attrs = trav.desc->attrs()) {
         // Note: This will likely be called once for each attr, making it
         // O(N^2) over the number of attrs.  If we want we could optimize for
         // large N by keeping a temporary map...somewhere
         //
         // First check direct attrs
        for (uint i = 0; i < attrs->n_attrs; i++) {
            auto attr = attrs->attr(i);
            if (attr->key == key) {
                trav_attr(trav, attr->acr(), key, op, cb);
                return true;
            }
        }
         // Then inherited attrs
        for (uint i = 0; i < attrs->n_attrs; i++) {
            auto attr = attrs->attr(i);
            auto acr = attr->acr();
            bool found = false;
            if (acr->attr_flags & ATTR_INHERIT) {
                 // Change op to modify so we don't clobber the other attrs of
                 // the inherited item.  Hopefully it won't matter, because
                 // inheriting through a non-addressable reference will be
                 // pretty slow no matter what.  Perhaps if we really wanted to
                 // optimize this, then in claim_keys we could build up a
                 // structure mirroring the inheritance diagram and follow it,
                 // instead of just keeping the flat list of keys.
                AccessOp inherit_op = op == ACR_WRITE ? ACR_MODIFY : op;
                trav_attr(
                    trav, acr, attr->key, inherit_op,
                    [&](const Traversal& child)
                {
                    found = ser_maybe_attr(child, key, op, cb);
                });
                if (found) return true;
            }
        }
         // Attr not found, fall through
    }
    if (auto attr_func = trav.desc->attr_func()) {
        if (Reference ref = attr_func->f(*trav.item, key)) {
            trav_attr_func(trav, std::move(ref), attr_func->f, key, op, cb);
            return true;
        }
         // Fallthrough
    }
    if (trav.desc->accepts_object()) {
         // Don't fallback to delegate if we support attributes but didn't find
         // one.
        return false;
    }
    else if (auto acr = trav.desc->delegate_acr()) {
        bool r;
        AccessOp del_op = op == ACR_WRITE ? ACR_MODIFY : op;
        trav_delegate(trav, acr, del_op, [&](const Traversal& child){
            r = ser_maybe_attr(child, key, op, cb);
        });
        return r;
    }
    else throw X::NoAttrs(trav_location(trav));
}
void in::ser_attr (
    const Traversal& trav, Str key, AccessOp op, TravCallback cb
) {
    if (!ser_maybe_attr(trav, key, op, cb)) {
        throw X::AttrNotFound(trav_location(trav), key);
    }
}

Reference item_maybe_attr (const Reference& item, Str key) {
    Reference r;
     // Is ACR_READ correct here?  Will we instead have to chain up the
     // reference from the start?
    trav_start(item, Location(Resource()), ACR_READ, [&](const Traversal& trav){
        ser_maybe_attr(trav, key, ACR_READ, [&](const Traversal& child){
            r = trav_reference(child);
        });
    });
    return r;
}
Reference item_attr (const Reference& item, Str key) {
    if (Reference r = item_maybe_attr(item, key)) {
        return r;
    }
    else throw X::AttrNotFound(Location(Resource()), key);
}

///// ELEM OPERATIONS

usize in::ser_get_length (const Traversal& trav) {
    usize len;
    if (auto acr = trav.desc->length_acr()) {
         // TODO: support other integral types besides usize
        acr->read(*trav.item, [&](const Mu& lv){
            len = reinterpret_cast<const usize&>(lv);
        });
    }
    else if (auto elems = trav.desc->elems()) {
        len = 0;
        for (usize i = 0; i < elems->n_elems; i++) {
            auto acr = elems->elem(i)->acr();
            if (acr->attr_flags & ATTR_INHERIT) {
                trav_elem(
                    trav, acr, i, ACR_READ, [&](const Traversal& child)
                {
                    len += ser_get_length(child);
                });
            }
            else len += 1;
        }
    }
    else if (auto acr = trav.desc->delegate_acr()) {
        trav_delegate(trav, acr, ACR_READ, [&](const Traversal& child){
            len = ser_get_length(child);
        });
    }
    else throw X::NoElems(trav_location(trav));
    return len;
}

usize item_get_length (const Reference& item) {
    usize len;
    trav_start(item, Location(Resource()), ACR_READ, [&](const Traversal& trav){
        len = ser_get_length(trav);
    });
    return len;
}

void in::ser_claim_length (const Traversal& trav, usize& claimed, usize len) {
    if (auto acr = trav.desc->length_acr()) {
        if (!(acr->accessor_flags & ACR_READONLY)) {
            usize remaining = (claimed <= len ? len - claimed : 0);
            acr->write(*trav.item, [&](Mu& lv){
                reinterpret_cast<usize&>(lv) = remaining;
            });
            claimed += remaining;
        }
        else {
             // For readonly length, just claim the returned length
            usize expected;
            acr->read(*trav.item, [&](const Mu& lv){
                expected = reinterpret_cast<const usize&>(lv);
            });
            claimed += expected;
        }
    }
    else if (auto elems = trav.desc->elems()) {
        for (usize i = 0; i < elems->n_elems; i++) {
            auto acr = elems->elem(i)->acr();
            if (claimed >= len && acr->attr_flags & ATTR_OPTIONAL) {
                continue;
            }
            if (acr->attr_flags & ATTR_INHERIT) {
                trav_elem(
                    trav, acr, i, ACR_WRITE, [&](const Traversal& child)
                {
                    ser_claim_length(child, claimed, len);
                });
            }
            else claimed += 1;
        }
    }
    else if (auto acr = trav.desc->delegate_acr()) {
        trav_delegate(trav, acr, ACR_WRITE, [&](const Traversal& child){
            ser_claim_length(child, claimed, len);
        });
    }
    else throw X::NoElems(trav_location(trav));
}

void in::ser_set_length (const Traversal& trav, usize len) {
    usize claimed = 0;
    ser_claim_length(trav, claimed, len);
    if (claimed > len) {
        throw X::TooShort(trav_location(trav), claimed, len);
    }
    if (claimed < len) {
        throw X::TooLong(trav_location(trav), claimed, len);
    }
}

void item_set_length (const Reference& item, usize len) {
    trav_start(
        item, Location(Resource()), ACR_WRITE, [&](const Traversal& trav)
    {
        ser_set_length(trav, len);
    });
}

bool in::ser_maybe_elem (
    const Traversal& trav, usize index, AccessOp op, TravCallback cb
) {
    if (auto elems = trav.desc->elems()) {
         // Including inheritance in this mix turns this from constant-time
         // to linear time, and thus serializing from linear time into
         // squared time.  We should probably have set a flag somewhere if
         // there no inherited elems so we can skip this loop.
        usize offset = 0;
        for (usize i = 0; i < elems->n_elems && offset <= index; i++) {
            auto acr = elems->elem(i)->acr();
            if (acr->attr_flags & ATTR_INHERIT) {
                bool found;
                 // Change op to modify so we don't clobber the other elems of
                 // the inherited item.
                AccessOp inherit_op = op == ACR_WRITE ? ACR_MODIFY : op;
                trav_elem(
                    trav, acr, i, inherit_op, [&](const Traversal& child)
                {
                    usize len = ser_get_length(child);
                    found = index - offset < len;
                    if (found) {
                         // Throw if inherited elem says it doesn't have an attr
                         // that's within its reported length.
                        ser_elem(child, index - offset, op, cb);
                    }
                    else offset += len;
                });
                if (found) return true;
            }
            else {
                if (offset == index) {
                    trav_elem(trav, acr, i, op, cb);
                    return true;
                }
                offset += 1;
            }
        }
         // Elem out of bounds, fall through to elem_func
    }
    if (auto elem_func = trav.desc->elem_func()) {
        if (Reference ref = elem_func->f(*trav.item, index)) {
            trav_elem_func(trav, std::move(ref), elem_func->f, index, op, cb);
            return true;
        }
         // Fall through
    }
    if (trav.desc->accepts_array()) {
         // Don't fall back to delegate if we support an array but didn't find
         // an element.
        return false;
    }
    else if (auto acr = trav.desc->delegate_acr()) {
        AccessOp del_op = op == ACR_WRITE ? ACR_MODIFY : op;
        bool found;
        trav_delegate(trav, acr, del_op, [&](const Traversal& child){
            found = ser_maybe_elem(child, index, op, cb);
        });
        return found;
    }
    else throw X::NoElems(trav_location(trav));
}
void in::ser_elem (
    const Traversal& trav, usize index, AccessOp op, TravCallback cb
) {
    if (!ser_maybe_elem(trav, index, op, cb)) {
        throw X::ElemNotFound(trav_location(trav), index);
    }
}
Reference item_maybe_elem (const Reference& item, usize index) {
    Reference r;
     // Is ACR_READ correct here?  Will we have to chain up the reference from
     // the start?
    trav_start(item, Location(Resource()), ACR_READ, [&](const Traversal& trav){
        ser_maybe_elem(trav, index, ACR_READ, [&](const Traversal& child){
            r = trav_reference(child);
        });
    });
    return r;
}
Reference item_elem (const Reference& item, usize index) {
    if (Reference r = item_maybe_elem(item, index)) {
        return r;
    }
    else throw X::ElemNotFound(Location(Resource()), index);
}

///// REFERENCES AND LOCATIONS
 // TODO: turn these into continuation-passing form like ser_*

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
