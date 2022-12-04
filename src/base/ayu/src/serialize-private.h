#pragma once
#include "../serialize.h"

#include "../reference.h"
#include "../resource.h"
#include "descriptors-private.h"
#include "location-private.h"
#include "traversal-private.h"

namespace ayu::in {

///// TO_TREE
Tree ser_to_tree (const Traversal&);

///// FROM_TREE
struct SwizzleOp {
    using FP = void(*)(Mu&, const Tree&);
    FP f;
    Reference item;
    Tree tree;
    Resource current_resource;
    SwizzleOp (FP f, const Reference& r, const Tree& t, Resource res) :
        f(f), item(r), tree(t), current_resource(res)
    { }
};
struct InitOp {
    using FP = void(*)(Mu&);
    FP f;
    Reference item;
    Resource current_resource;
    InitOp (FP f, const Reference& r, Resource res) :
        f(f), item(r), current_resource(res)
    { }
};
inline std::vector<SwizzleOp> swizzle_ops;
inline std::vector<InitOp> init_ops;

void ser_do_swizzles ();
void ser_do_inits ();
void ser_from_tree (const Traversal&, const Tree&);

///// ATTR OPERATIONS
 // StrVector behaves just like a std::vector<Str>, but has extra storage in
 // case it needs to take ownership of any strings.  Ideally, the storage will
 // be unused and remain empty.
struct OwnedStringNode {
    std::string s;
    std::unique_ptr<OwnedStringNode> next;
};
struct StrVector : std::vector<Str> {
    using std::vector<Str>::vector;
     // Moving a std::string might invalidate its Str, so we can't keep them in
     // a resizable std::vector.  Just use a singly-linked list because we don't
     // actually need to DO anything with these, we just need to keep them
     // around.
    std::unique_ptr<OwnedStringNode> owned_strings;
};

 // Implement get_keys by adding keys to a StrVector
void ser_collect_key_str (StrVector&, Str);
void ser_collect_key_string (StrVector&, String&&);
void ser_collect_keys (const Traversal&, StrVector&);

 // Implement set_keys by removing keys from a std::vector<Str>
bool ser_claim_key (std::vector<Str>&, Str);
void ser_claim_keys (const Traversal&, std::vector<Str>&, bool optional);
void ser_set_keys (const Traversal&, std::vector<Str>&&);

 // If the attr isn't found, returns false and doesn't call the callback
bool ser_maybe_attr (const Traversal&, Str, AccessOp, TravCallback);
 // Throws if the attr isn't found
void ser_attr (const Traversal&, Str, AccessOp, TravCallback);

 ///// Elem operations
usize ser_get_length (const Traversal&);
 // Implement set_length by counting up used length
void ser_claim_length (const Traversal&, usize& claimed, usize len);
void ser_set_length (const Traversal&, usize);

 // If elem is out of range, returns false and doesn't call the callback
bool ser_maybe_elem (const Traversal&, usize, AccessOp, TravCallback);
 // Throws if elem is out of bounds
void ser_elem (const Traversal&, usize, AccessOp, TravCallback);

 ///// Scanning operations

 // Scan all data visible to ayu.
void recursive_scan_universe (
    Callback<void(const Reference&, Location)> cb
);

 // Scan only a particular resource.  Silently does nothing if the resource is
 // UNLOADED.  TODO: Should it throw instead?
void recursive_scan_resource (
    Resource res,
    Callback<void(const Reference&, Location)> cb
);

 // Scan only data under a given reference.  base_location should be the
 // location of base_item.
void recursive_scan (
    const Reference& base_item,
    Location base_location,
    Callback<void(const Reference&, Location)> cb
);


} // namespace ayu::in
