#pragma once
#include "../serialize.h"

#include "../reference.h"
#include "../resource.h"
#include "descriptors-private.h"
#include "location-private.h"

namespace ayu::in {

 ///// to_tree
Tree inner_to_tree (
    const DescriptionPrivate* desc, const Mu& item, TempLocation* loc
);

 ///// from_tree
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

void do_swizzles ();
void do_inits ();
void inner_from_tree (
    const DescriptionPrivate* desc, Mu& item, const Tree& tree,
    const Reference* unaddressable_ref, TempLocation* loc
);

 ///// Attr operations
struct OwnedStringNode {
    std::string s;
    std::unique_ptr<OwnedStringNode> next;
};
 // StrVector behaves just like a std::vector<Str>, but has extra storage in
 // case it needs to take ownership of any strings.  Ideally, the storage will
 // be unused and remain empty.
struct StrVector : std::vector<Str> {
    using std::vector<Str>::vector;
     // Moving a std::string might invalidate its Str, so we can't keep them in
     // a resizable std::vector.  Just use a singly-linked list because we don't
     // actually need to DO anything with these, we just need to keep them
     // around.
    std::unique_ptr<OwnedStringNode> owned_strings;
};

void collect_key_str (StrVector& ks, Str k);
void collect_key_string (StrVector& ks, String&& k);

void collect_keys (
    const DescriptionPrivate* desc, const Mu& item, StrVector& ks,
    const Reference* unaddressable_ref, TempLocation* loc
);

bool claim_key (std::vector<Str>& ks, Str k);
void item_claim_keys (const Reference& item, std::vector<Str>& ks, bool optional);

Reference inner_attr (
    const DescriptionPrivate* desc, const Mu& item, Str k,
    const Reference* unaddressable_ref, TempLocation* loc
);

 ///// Elem operations
void item_claim_length (const Reference& item, usize& claimed, usize len);

} // namespace ayu::in
