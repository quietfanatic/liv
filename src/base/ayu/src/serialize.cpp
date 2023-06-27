#include "serialize-private.h"

#include <cassert>
#include "../describe.h"

namespace ayu {
using namespace in;

///// TO_TREE
namespace in {
    static uint64 diagnostic_serialization = 0;
}
DiagnosticSerialization::DiagnosticSerialization () {
    diagnostic_serialization += 1;
}
DiagnosticSerialization::~DiagnosticSerialization () {
    expect(diagnostic_serialization > 0);
    diagnostic_serialization -= 1;
}

Tree in::ser_to_tree (const Traversal& trav) {
    try {
        if (auto to_tree = trav.desc->to_tree()) {
            return to_tree->f(*trav.address);
        }
        if (auto values = trav.desc->values()) {
            for (uint i = 0; i < values->n_values; i++) {
                Tree r = values->value(i)->value_to_tree(values, *trav.address);
                if (r.has_value()) return r;
            }
        }
        switch (trav.desc->preference()) {
            case Description::PREFER_OBJECT: {
                TreeObject o;
                UniqueArray<AnyString> ks;
                ser_collect_keys(trav, ks);
                o.reserve(ks.size());
                for (auto& k : ks) {
                    ser_attr(
                        trav, k, ACR_READ, [&](const Traversal& child)
                    {
                         // Don't serialize readonly attributes, because they
                         // can't be deserialized.
                        if (!child.readonly) {
                            Tree t = ser_to_tree(child);
                             // Get flags from acr
                            if (child.op == ATTR) {
                                t.flags |= child.acr->tree_flags();
                            }
                             // It's okay to move k even though the traversal
                             // stack has a pointer to it, because this is the
                             // last thing that happens before ser_attr returns.
                            o.emplace_back_expect_capacity(move(k), move(t));
                        }
                    });
                }
                 // Evil optimization: We know we've cleared out the array, so
                 // manually zero it out to prevent its destructor from
                 // iterating over it again
                // ks.materialize_size(0);
                return Tree(move(o));
            }
            case Description::PREFER_ARRAY: {
                TreeArray a;
                usize l = ser_get_length(trav);
                a.reserve(l);
                for (usize i = 0; i < l; i++) {
                    ser_elem(
                        trav, i, ACR_READ, [&](const Traversal& child)
                    {
                         // Readonly elems are problematic, because they can't
                         // just be skipped without changing the order of other
                         // elems.  We should probably just forbid them.
                         // TODO: do that?  Or will that interfere with
                         // exception printing?
                        Tree t = ser_to_tree(child);
                        if (child.op == ATTR) {
                            t.flags |= child.acr->tree_flags();
                        }
                        a.emplace_back_expect_capacity(move(t));
                    });
                }
                return Tree(move(a));
            }
            default: {
                if (auto acr = trav.desc->delegate_acr()) {
                    Tree r;
                    trav_delegate(
                        trav, acr, ACR_READ, [&](const Traversal& child)
                    {
                        r = ser_to_tree(child);
                        r.flags |= acr->tree_flags();
                    });
                    return r;
                }
                else if (trav.desc->values()) {
                    throw X<NoNameForValue>(trav_location(trav), trav.desc);
                }
                else throw X<CannotToTree>(trav_location(trav), trav.desc);
            }
        }
    }
    catch (const Error& e) {
        if (diagnostic_serialization) {
            return Tree(std::current_exception());
        }
        else throw;
    }
}
Tree item_to_tree (const Reference& item, LocationRef loc) {
    Tree r;
    trav_start(item, loc, false, ACR_READ, [&](const Traversal& trav){
        r = ser_to_tree(trav);
    });
    return r;
}

///// FROM_TREE
void in::ser_from_tree (const Traversal& trav, TreeRef tree) {
     // If description has a from_tree, just use that.
    if (auto from_tree = trav.desc->from_tree()) {
        from_tree->f(*trav.address, tree);
        goto done;
    }
     // Now the behavior depends on what kind of tree we've been given
    switch (tree->form) {
        case OBJECT: {
            if (trav.desc->accepts_object()) {
                expect(tree->rep == REP_OBJECT);
                auto o = TreeObjectSlice(*tree);
                UniqueArray<AnyString> ks; ks.reserve(o.size());
                for (auto& p : o) {
                    ks.emplace_back_expect_capacity(p.first);
                }
                ser_set_keys(trav, move(ks));
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
                expect(tree->rep == REP_ARRAY);
                auto a = TreeArraySlice(*tree);
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
            std::rethrow_exception(std::exception_ptr(*tree));
        }
        default: {
             // All other tree types support the values descriptor
            if (auto values = trav.desc->values()) {
                for (uint i = 0; i < values->n_values; i++) {
                    if (Mu* v = values->value(i)->tree_to_value(tree)) {
                        values->assign(*trav.address, *v);
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
     // Go through maybe a little too much effort to figure out what went wrong.
    [[unlikely]]
    if (tree->form == OBJECT &&
        (trav.desc->values() || trav.desc->accepts_array())
    ) {
        throw X<InvalidForm>(trav_location(trav), trav.desc, tree);
    }
    else if (tree->form == ARRAY &&
        (trav.desc->values() || trav.desc->accepts_object())
    ) {
        throw X<InvalidForm>(trav_location(trav), trav.desc, tree);
    }
    else if (trav.desc->accepts_array() || trav.desc->accepts_object()) {
        throw X<InvalidForm>(trav_location(trav), trav.desc, tree);
    }
    else if (trav.desc->values()) {
        throw X<NoValueForName>(trav_location(trav), trav.desc, tree);
    }
    else {
        throw X<CannotFromTree>(trav_location(trav), trav.desc);
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
            IFTContext::current->swizzle_ops.emplace_back(
                swizzle->f, ref, tree
            );
        }
        if (init) {
            IFTContext::current->init_ops.emplace_back(init->f, ref);
        }
    }
}

void in::IFTContext::do_swizzles () {
     // Swizzling might add more swizzle ops; this will happen if we're
     // swizzling a pointer which points to a separate resource; that resource
     // will be load()ed in op.f.
    while (!swizzle_ops.empty()) {
         // Explicitly assign to clear swizzle_ops
        auto swizzles = move(swizzle_ops);
        for (auto& op : swizzles) {
            trav_start(
                op.item, op.loc, false, ACR_MODIFY, [&](const Traversal& trav)
            {
                op.f(*trav.address, op.tree);
            });
        }
    }
}
void in::IFTContext::do_inits () {
     // Initting might add some more init ops.  It'd be weird, but it's allowed
     // for an init() to load another resource.
    while (!init_ops.empty()) {
        auto inits = move(init_ops);
        for (auto& op : inits) {
            trav_start(
                op.item, op.loc, false, ACR_MODIFY, [&](const Traversal& trav)
            {
                op.f(*trav.address);
            });
             // Initting might even add more swizzle ops.
            do_swizzles();
        }
    }
}

IFTContext* IFTContext::current = null;

void item_from_tree (
    const Reference& item, TreeRef tree, LocationRef loc,
    ItemFromTreeFlags flags
) {
     // TODO: Replace with expect()
    if (tree->form == UNDEFINED) {
        throw X<GenericError>("Undefined tree passed to item_from_tree");
    }
    if (flags & DELAY_SWIZZLE && IFTContext::current) {
         // Delay swizzle and inits to the outer item_from_tree call.  Basically
         // this just means keep the current context instead of making a new
         // one.
        trav_start(item, loc, false, ACR_WRITE, [&](const Traversal& trav){
            ser_from_tree(trav, tree);
        });
    }
    else {
        IFTContext context;
        trav_start(item, loc, false, ACR_WRITE, [&](const Traversal& trav){
            ser_from_tree(trav, tree);
        });
        context.do_swizzles();
        context.do_inits();
    }
}

///// ATTR OPERATIONS
void in::ser_collect_key (UniqueArray<AnyString>& ks, AnyString&& k) {
     // This'll end up being N^2.  TODO: Test whether including an unordered_set
     // would speed this up (probably not).
    for (auto ksk : ks) if (k == ksk) return;
    ks.emplace_back(move(k));
}

void in::ser_collect_keys (const Traversal& trav, UniqueArray<AnyString>& ks) {
    if (auto acr = trav.desc->keys_acr()) {
        Type keys_type = acr->type(trav.address);
         // Compare Type not std::type_info, since std::type_info can require a
         // string comparison.
        if (keys_type == Type::CppType<AnyArray<AnyString>>()) {
             // Optimize for AnyArray<AnyString>
            acr->read(*trav.address, [&](Mu& v){
                auto& ksv = reinterpret_cast<const AnyArray<AnyString>&>(v);
                for (auto& k : ksv) {
                    ser_collect_key(ks, AnyString(k));
                }
            });
        }
        else {
             // General case, any type that serializes to an array of strings.
            acr->read(*trav.address, [&](Mu& ksv){
                 // We might be able to optimize this more, but it's not that
                 // important.
                auto tree = item_to_tree(Pointer(&ksv, keys_type));
                if (tree.form != ARRAY) {
                    throw X<InvalidKeysType>(trav_location(trav), trav.desc, keys_type);
                }
                for (const Tree& e : TreeArraySlice(tree)) {
                    if (e.form != STRING) {
                        throw X<InvalidKeysType>(trav_location(trav), trav.desc, keys_type);
                    }
                    ser_collect_key(ks, AnyString(move(e)));
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
            else ser_collect_key(ks, attr->key);
        }
    }
    else if (auto acr = trav.desc->delegate_acr()) {
        trav_delegate(trav, acr, ACR_READ, [&](const Traversal& child){
            ser_collect_keys(child, ks);
        });
    }
    else throw X<NoAttrs>(trav_location(trav), trav.desc);
}

AnyArray<AnyString> item_get_keys (
    const Reference& item, LocationRef loc
) {
    UniqueArray<AnyString> ks;
    trav_start(item, loc, false, ACR_READ, [&](const Traversal& trav){
        ser_collect_keys(trav, ks);
    });
    return ks;
}

bool in::ser_claim_key (UniqueArray<AnyString>& ks, Str k) {
     // This algorithm overall is O(N^3), we may be able to speed it up by
     // setting a flag if there are no inherited attrs, or maybe by using an
     // unordered_set?
     // TODO: Just use a bool array for claiming instead of erasing from
     // the vector?
    for (usize i = 0; i < ks.size(); ++i) {
        if (ks[i] == k) {
            ks.erase(i);
            return true;
        }
    }
    return false;
}

void in::ser_claim_keys (
    const Traversal& trav,
    UniqueArray<AnyString>& ks,
    bool optional
) {
    if (auto acr = trav.desc->keys_acr()) {
        Type keys_type = acr->type(trav.address);
        if (!(acr->accessor_flags & ACR_READONLY)) {
            if (keys_type == Type::CppType<AnyArray<AnyString>>()) {
                 // Optimize for AnyArray<AnyString>
                acr->write(*trav.address, [&](Mu& ksv){
                    reinterpret_cast<AnyArray<AnyString>&>(ksv) = ks;
                });
            }
            else {
                 // General case: call item_from_tree on the keys.  This will
                 // be slow.
                UniqueArray<Tree> a (ks.size());
                for (usize i = 0; i < ks.size(); i++) {
                    a[i] = Tree(ks[i]);
                }
                acr->write(*trav.address, [&](Mu& ksv){
                    item_from_tree(
                        Pointer(&ksv, keys_type), Tree(move(a))
                    );
                });
            }
            ks.clear();
        }
        else {
             // For readonly keys, get the keys and compare them.
             // TODO: This can probably be optimized more
            UniqueArray<AnyString> got_ks;
            ser_collect_keys(trav, got_ks);
            for (auto& k : got_ks) {
                if (ser_claim_key(ks, k)) {
                     // If any of the keys are present, it makes this item no
                     // longer optional.
                    optional = false;
                }
                else if (!optional) {
                    throw X<MissingAttr>(trav_location(trav), trav.desc, k);
                }
            }
            return;
        }
    }
    else if (auto attrs = trav.desc->attrs()) {
         // Prioritize direct attrs
         // I don't think it's possible for n_attrs to be large enough to
         // overflow the stack...right?  The max description size is 64K and an
         // attr always consumes at least 14 bytes, so the max n_attrs is
         // something like 4500.  TODO: enforce a reasonable max n_attrs in
         // descriptors-internal.h.
        bool claimed_inherited [attrs->n_attrs] = {};
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
                throw X<MissingAttr>(trav_location(trav), trav.desc, attr->key);
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
    else throw X<NoAttrs>(trav_location(trav), trav.desc);
}

void in::ser_set_keys (const Traversal& trav, UniqueArray<AnyString>&& ks) {
    ser_claim_keys(trav, ks, false);
    if (!ks.empty()) {
        throw X<UnwantedAttr>(trav_location(trav), trav.desc, ks[0]);
    }
}

void item_set_keys (
    const Reference& item, AnyArray<AnyString> ks,
    LocationRef loc
) {
    trav_start(item, loc, false, ACR_WRITE, [&](const Traversal& trav){
        ser_set_keys(trav, move(ks));
    });
}

bool in::ser_maybe_attr (
    const Traversal& trav, const AnyString& key,
    AccessMode mode, TravCallbackRef cb
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
                trav_attr(trav, attr->acr(), key, mode, cb);
                return true;
            }
        }
         // Then inherited attrs
        for (uint i = 0; i < attrs->n_attrs; i++) {
            auto attr = attrs->attr(i);
            auto acr = attr->acr();
            bool found = false;
            if (acr->attr_flags & ATTR_INHERIT) {
                 // Change mode to modify so we don't clobber the other attrs of
                 // the inherited item.  Hopefully it won't matter, because
                 // inheriting through a non-addressable reference will be
                 // pretty slow no matter what.  Perhaps if we really wanted to
                 // optimize this, then in claim_keys we could build up a
                 // structure mirroring the inheritance diagram and follow it,
                 // instead of just keeping the flat list of keys.
                AccessMode inherit_mode = mode == ACR_WRITE ? ACR_MODIFY : mode;
                trav_attr(
                    trav, acr, attr->key, inherit_mode,
                    [&](const Traversal& child)
                {
                    found = ser_maybe_attr(child, key, mode, cb);
                });
                if (found) return true;
            }
        }
        return false;
    }
    else if (auto attr_func = trav.desc->attr_func()) {
        if (Reference ref = attr_func->f(*trav.address, key)) {
            trav_attr_func(trav, move(ref), attr_func->f, key, mode, cb);
            return true;
        }
        return false;
    }
    else if (auto acr = trav.desc->delegate_acr()) {
        bool r;
        AccessMode del_mode = mode == ACR_WRITE ? ACR_MODIFY : mode;
        trav_delegate(trav, acr, del_mode, [&](const Traversal& child){
            r = ser_maybe_attr(child, key, mode, cb);
        });
        return r;
    }
    else throw X<NoAttrs>(trav_location(trav), trav.desc);
}
void in::ser_attr (
    const Traversal& trav, const AnyString& key, AccessMode mode, TravCallbackRef cb
) {
    if (!ser_maybe_attr(trav, key, mode, cb)) {
        throw X<AttrNotFound>(trav_location(trav), trav.desc, key);
    }
}

Reference item_maybe_attr (
    const Reference& item, AnyString key, LocationRef loc
) {
    Reference r;
     // Is ACR_READ correct here?  Will we instead have to chain up the
     // reference from the start?
    trav_start(item, loc, false, ACR_READ, [&](const Traversal& trav){
        ser_maybe_attr(trav, key, ACR_READ, [&](const Traversal& child){
            r = trav_reference(child);
        });
    });
    return r;
}
Reference item_attr (const Reference& item, AnyString key, LocationRef loc) {
    if (Reference r = item_maybe_attr(item, key)) {
        return r;
    }
    else throw X<AttrNotFound>(loc, item.type(), move(key));
}

///// ELEM OPERATIONS

usize in::ser_get_length (const Traversal& trav) {
    if (auto acr = trav.desc->length_acr()) {
        usize len;
         // TODO: support other integral types besides usize
        acr->read(*trav.address, [&](Mu& lv){
            len = reinterpret_cast<const usize&>(lv);
        });
        return len;
    }
    else if (auto elems = trav.desc->elems()) {
        return elems->n_elems;
    }
    else if (auto acr = trav.desc->delegate_acr()) {
        usize len;
        trav_delegate(trav, acr, ACR_READ, [&](const Traversal& child){
            len = ser_get_length(child);
        });
        return len;
    }
    else throw X<NoElems>(trav_location(trav), trav.desc);
}

usize item_get_length (const Reference& item, LocationRef loc) {
    usize len;
    trav_start(item, loc, false, ACR_READ, [&](const Traversal& trav){
        len = ser_get_length(trav);
    });
    return len;
}

void in::ser_set_length (const Traversal& trav, usize len) {
    if (auto acr = trav.desc->length_acr()) {
        if (!(acr->accessor_flags & ACR_READONLY)) {
            acr->write(*trav.address, [&](Mu& lv){
                reinterpret_cast<usize&>(lv) = len;
            });
        }
        else {
             // For readonly length, just check that the provided length matches
            usize expected;
            acr->read(*trav.address, [&](Mu& lv){
                expected = reinterpret_cast<const usize&>(lv);
            });
            if (len != expected) {
                throw X<WrongLength>{
                    trav_location(trav), trav.desc, expected, expected, len
                };
            }
        }
    }
    else if (auto elems = trav.desc->elems()) {
        usize min = elems->n_elems;
         // Scan backwards for optional elements
        while (min > 0) {
            if (elems->elem(min-1)->acr()->attr_flags & ATTR_OPTIONAL) {
                min -= 1;
            }
            else break;
        }
        if (len < min || len > elems->n_elems) {
            throw X<WrongLength>(trav_location(trav), trav.desc, min, elems->n_elems, len);
        }
    }
    else if (auto acr = trav.desc->delegate_acr()) {
        trav_delegate(trav, acr, ACR_WRITE, [&](const Traversal& child){
            ser_set_length(child, len);
        });
    }
    else throw X<NoElems>(trav_location(trav), trav.desc);
}

void item_set_length (const Reference& item, usize len, LocationRef loc) {
    trav_start(item, loc, false, ACR_WRITE, [&](const Traversal& trav){
        ser_set_length(trav, len);
    });
}

bool in::ser_maybe_elem (
    const Traversal& trav, usize index, AccessMode mode, TravCallbackRef cb
) {
    if (auto elems = trav.desc->elems()) {
        if (index < elems->n_elems) {
            auto acr = elems->elem(index)->acr();
            trav_elem(trav, acr, index, mode, cb);
            return true;
        }
        return false;
    }
    else if (auto elem_func = trav.desc->elem_func()) {
        if (Reference ref = elem_func->f(*trav.address, index)) {
            trav_elem_func(trav, move(ref), elem_func->f, index, mode, cb);
            return true;
        }
        return false;
    }
    else if (auto acr = trav.desc->delegate_acr()) {
        AccessMode del_mode = mode == ACR_WRITE ? ACR_MODIFY : mode;
        bool found;
        trav_delegate(trav, acr, del_mode, [&](const Traversal& child){
            found = ser_maybe_elem(child, index, mode, cb);
        });
        return found;
    }
    else throw X<NoElems>(trav_location(trav), trav.desc);
}
void in::ser_elem (
    const Traversal& trav, usize index, AccessMode mode, TravCallbackRef cb
) {
    if (!ser_maybe_elem(trav, index, mode, cb)) {
        throw X<ElemNotFound>(trav_location(trav), trav.desc, index);
    }
}
Reference item_maybe_elem (
    const Reference& item, usize index, LocationRef loc
) {
    Reference r;
     // TODO: We probably don't need to set up a whole traversal stack for this,
     // now that we've removed inherited elems.
    trav_start(item, loc, false, ACR_READ, [&](const Traversal& trav){
        ser_maybe_elem(trav, index, ACR_READ, [&](const Traversal& child){
            r = trav_reference(child);
        });
    });
    return r;
}
Reference item_elem (const Reference& item, usize index, LocationRef loc) {
    if (Reference r = item_maybe_elem(item, index)) {
        return r;
    }
    else throw X<ElemNotFound>(loc, item.type(), index);
}

///// MISC

Location current_location () {
    if (current_traversal) {
        return trav_location(*current_traversal);
    }
    else return Location();
}

} using namespace ayu;

AYU_DESCRIBE(ayu::SerError,
    elems(
        elem(base<Error>(), inherit),
        elem(&SerError::location),
        elem(&SerError::type)
    )
)

AYU_DESCRIBE(ayu::CannotToTree,
    delegate(base<SerError>())
)
AYU_DESCRIBE(ayu::CannotFromTree,
    delegate(base<SerError>())
)
AYU_DESCRIBE(ayu::InvalidForm,
    delegate(base<SerError>())
)
AYU_DESCRIBE(ayu::NoNameForValue,
    delegate(base<SerError>())
)
AYU_DESCRIBE(ayu::NoValueForName,
    elems(
        elem(base<SerError>(), inherit),
        elem(&NoValueForName::tree)
    )
)
AYU_DESCRIBE(ayu::MissingAttr,
    elems(
        elem(base<SerError>(), inherit),
        elem(&MissingAttr::key)
    )
)
AYU_DESCRIBE(ayu::UnwantedAttr,
    elems(
        elem(base<SerError>(), inherit),
        elem(&UnwantedAttr::key)
    )
)
AYU_DESCRIBE(ayu::WrongLength,
    attrs(
        attr("ayu::SerError", base<SerError>(), inherit),
        attr("min", &WrongLength::min),
        attr("max", &WrongLength::max),
        attr("got", &WrongLength::got)
    )
)
AYU_DESCRIBE(ayu::NoAttrs,
    delegate(base<SerError>())
)
AYU_DESCRIBE(ayu::NoElems,
    delegate(base<SerError>())
)
AYU_DESCRIBE(ayu::AttrNotFound,
    elems(
        elem(base<SerError>(), inherit),
        elem(&AttrNotFound::key)
    )
)
AYU_DESCRIBE(ayu::ElemNotFound,
    elems(
        elem(base<SerError>(), inherit),
        elem(&ElemNotFound::index)
    )
)
AYU_DESCRIBE(ayu::InvalidKeysType,
    elems(
        elem(base<SerError>(), inherit),
        elem(&InvalidKeysType::type)
    )
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

    struct ElemsTest {
        std::vector<int> xs;
    };

     // Test usage of keys() with type std::vector<std::string>
    struct AttrsTest {
        std::unordered_map<std::string, int> xs;
    };
     // Test usage of keys() with type AnyArray<AnyString>
    struct AttrsTest2 {
        std::unordered_map<AnyString, int> xs;
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
    struct InternalRefTest {
        int a;
        int b;
        int* p;
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
    keys(mixed_funcs<std::vector<std::string>>(
        [](const AttrsTest& v){
            std::vector<std::string> r;
            for (auto& p : v.xs) {
                r.emplace_back(p.first);
            }
            return r;
        },
        [](AttrsTest& v, const std::vector<std::string>& ks){
            v.xs.clear();
            for (auto& k : ks) {
                v.xs.emplace(k, 0);
            }
        }
    )),
    attr_func([](AttrsTest& v, AnyString k){
        return Reference(&v.xs.at(k));
    })
)
AYU_DESCRIBE(ayu::test::AttrsTest2,
    keys(mixed_funcs<AnyArray<AnyString>>(
        [](const AttrsTest2& v){
            AnyArray<AnyString> r;
            for (auto& p : v.xs) {
                r.emplace_back(p.first);
            }
            return r;
        },
        [](AttrsTest2& v, const AnyArray<AnyString>& ks){
            v.xs.clear();
            for (auto& k : ks) {
                v.xs.emplace(k, 0);
            }
        }
    )),
    attr_func([](AttrsTest2& v, AnyString k){
        return Reference(&v.xs.at(k));
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
AYU_DESCRIBE(ayu::test::InternalRefTest,
    attrs(
        attr("a", &InternalRefTest::a),
        attr("b", &InternalRefTest::b),
        attr("p", &InternalRefTest::p)
    )
)

static tap::TestSet tests ("base/ayu/serialize", []{
    using namespace tap;
    ok(get_description_for_type_info(typeid(MemberTest)), "Description was registered");

    auto try_to_tree = [](Reference item, Str tree, Str name){
        try_is<Tree, Tree>(
            [&item]{ return item_to_tree(item); },
            tree_from_string(tree),
            name
        );
    };

    auto ttt = ToTreeTest{5};
    try_to_tree(&ttt, "5", "item_to_tree works with to_tree descriptor");

    ValuesTest vtt = VTA;
    try_to_tree(&vtt, "\"vta\"", "item_to_tree works with string value");
    vtt = VTNULL;
    try_to_tree(&vtt, "null", "item_to_tree works with null value");
    vtt = VTZERO;
    try_to_tree(&vtt, "0", "item_to_tree works with int value");
    vtt = VTNAN;
    try_to_tree(&vtt, "+nan", "item_to_tree works with double value");
    vtt = ValuesTest(999);
    doesnt_throw([&]{ item_from_string(&vtt, "\"vta\""); });
    is(vtt, VTA, "item_from_tree works with string value");
    doesnt_throw([&]{ item_from_string(&vtt, "null"); });
    is(vtt, VTNULL, "item_from_tree works with null value");
    doesnt_throw([&]{ item_from_string(&vtt, "0"); });
    is(vtt, VTZERO, "item_from_tree works with int value");
    doesnt_throw([&]{ item_from_string(&vtt, "+nan"); });
    is(vtt, VTNAN, "item_from_tree works with double value");

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
    throws<MissingAttr>([&]{
        item_from_string(&mt, "{a:16}");
    }, "item_from_tree throws on missing attr with attrs descriptor");
    throws<WrongForm>([&]{
        item_from_string(&mt, "{a:41 b:foo}");
    }, "item_from_tree throws WrongForm when attr has wrong form");
    throws<CantRepresent>([&]{
        item_from_string(&mt, "{a:41 b:4.3}");
    }, "item_from_tree throws CantRepresent when int attr isn't integer");
    throws<InvalidForm>([&]{
        item_from_string(&mt, "[54 43]");
    }, "item_from_tree throws InvalidForm when trying to make attrs object from array");
    throws<UnwantedAttr>([&]{
        item_from_string(&mt, "{a:0 b:1 c:60}");
    }, "item_from_tree throws on extra attr");

    auto bt = BaseTest{{-1, -2}, -3};
    Tree btt = item_to_tree(&bt);
    is(btt, tree_from_string("{MemberTest:{a:-1,b:-2} c:-3}"), "item_to_tree with base attr");
    Tree from_tree_bt1 = tree_from_string("{c:-4,MemberTest:{a:-5,b:-6}}");
    item_from_tree(&bt, from_tree_bt1);
    is(bt.b, -6, "item_from_tree with base attr");
    throws<MissingAttr>([&]{
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
    throws<MissingAttr>([&]{
        item_from_tree(&iot, tree_from_string("{d:34 MemberTest:{a:56 b:67}}"));
    }, "Optional inherited attrs need either all or no attrs");
    todo(1);
    throws<MissingAttr>([&]{
        item_from_tree(&iot, tree_from_string("{d:34 c:78}"));
    }, "Optional inherited attrs need either all or no attrs (2)");

    auto et = ElemTest{0.5, 1.5, 2.5};
    Tree ett = item_to_tree(&et);
    is(ett, tree_from_string("[0.5 1.5 2.5]"), "item_to_tree with elems descriptor");
    Tree from_tree_et1 = tree_from_string("[3.5 4.5 5.5]");
    item_from_tree(&et, from_tree_et1);
    is(et.y, 4.5, "item_from_tree with elems descriptor");
    throws<WrongLength>([&]{
        item_from_string(&et, "[6.5 7.5]");
    }, "item_from_tree throws on too short array with elems descriptor");
    throws<WrongLength>([&]{
        item_from_string(&et, "[6.5 7.5 8.5 9.5]");
    }, "item_from_tree throws on too long array with elems descriptor");
    throws<InvalidForm>([&]{
        item_from_string(&et, "{x:1.1 y:2.2}");
    }, "item_from_tree throws InvalidForm when trying to make elems thing from object");

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
    try_to_tree(&est, "[1 3 6 10 15 0 0 0 99]", "item_to_tree with length and elem_func");
    doesnt_throw([&]{
        item_from_string(&est, "[5 2 0 4]");
    }, "item_from_tree with length and elem_func doesn't throw");
    is(est.xs.at(3), 4, "item_from_tree works with elem_func");

    auto ast = AttrsTest{{{"a", 11}, {"b", 22}}};
    auto keys = item_get_keys(&ast);
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
    auto ks = std::vector<AnyString>{"c", "d"};
    item_set_keys(&ast, Slice<AnyString>(ks));
    is(ast.xs.find("a"), ast.xs.end(), "item_set_keys removed key");
    is(ast.xs.at("c"), 0, "item_set_keys added key");
    doesnt_throw([&]{
        item_attr(&ast, "d").write_as<int>([](int& v){ v = 999; });
    }, "item_attr and Reference::write_as");
    is(ast.xs.at("d"), 999, "writing to attr works");
    try_to_tree(&ast, "{c:0,d:999}", "item_to_tree with keys and attr_func");
    doesnt_throw([&]{
        item_from_string(&ast, "{e:88,f:34}");
    }, "item_from_tree with keys and attr_func doesn't throw");
    is(ast.xs.at("f"), 34, "item_from_tree works with attr_func");

    auto ast2 = AttrsTest2{{{"a", 11}, {"b", 22}}};
    keys = item_get_keys(&ast2);
    is(keys.size(), 2u, "item_get_keys (size)");
    ok((keys[0] == "a" && keys[1] == "b") || (keys[0] == "b" && keys[1] == "a"),
        "item_get_keys (contents)"
    );
    answer = 0;
    doesnt_throw([&]{
        item_attr(&ast2, "b").read_as<int>([&](const int& v){ answer = v; });
    }, "item_attr and Reference::read_as");
    is(answer, 22, "item_attr gives correct answer");
    throws<std::out_of_range>([&]{
        item_attr(&ast2, "c");
    }, "item_attr can throw on missing key (from user-defined function)");
    ks = std::vector<AnyString>{"c", "d"};
    item_set_keys(&ast2, Slice<AnyString>(ks));
    is(ast2.xs.find("a"), ast2.xs.end(), "item_set_keys removed key");
    is(ast2.xs.at("c"), 0, "item_set_keys added key");
    doesnt_throw([&]{
        item_attr(&ast2, "d").write_as<int>([](int& v){ v = 999; });
    }, "item_attr and Reference::write_as");
    is(ast2.xs.at("d"), 999, "writing to attr works");
    try_to_tree(&ast2, "{c:0,d:999}", "item_to_tree with keys and attr_func");
    doesnt_throw([&]{
        item_from_string(&ast2, "{e:88,f:34}");
    }, "item_from_tree with keys and attr_func doesn't throw");
    is(ast2.xs.at("f"), 34, "item_from_tree works with attr_func");

    auto dt = DelegateTest{{4, 5, 6}};
    try_to_tree(&dt, "[4 5 6]", "item_to_tree with delegate");
    doesnt_throw([&]{
        item_from_string(&dt, "[7 8 9]");
    });
    is(dt.et.y, 8, "item_from_tree with delegate");
    is(item_elem(&dt, 2).address_as<float>(), &dt.et.z, "item_elem works with delegate");

    std::vector<ToTreeTest> tttv {{444}, {333}};
    try_to_tree(&tttv, "[444 333]", "template describe on std::vector works");
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
    try_to_tree(&set, "[0xa 0xb]", "Can use elems() on scalar type (to_tree)");
    doesnt_throw([&]{
        item_from_string(&set, "[0xc 0xd]");
    });
    is(set, ScalarElemTest(0xcd), "Can use elems() on scalar type (from_tree)");

    todo([&]{
        InternalRefTest irt = {3, 4, null};
        irt.p = &irt.a;
        try_to_tree(&irt, "{a:3 b:4 p:#a}", "Can serialize item with internal refs");
        doesnt_throw([&]{
            item_from_string(&irt, "{a:5 b:6 p:#b}");
        });
        is(irt.p, &irt.b, "Can deserialize item with internal refs");
    }, "internal references without resource system nyi");
    done_testing();
});
#endif
