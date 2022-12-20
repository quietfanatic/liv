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
    Location loc;
    SwizzleOp (FP f, const Reference& r, const Tree& t) :
        f(f), item(r), tree(t), loc(trav_location(*current_traversal))
    { }
};
struct InitOp {
    using FP = void(*)(Mu&);
    FP f;
    Reference item;
    Location loc;
    InitOp (FP f, const Reference& r) :
        f(f), item(r), loc(trav_location(*current_traversal))
    { }
};
struct IFTContext {
    static IFTContext* current;
    IFTContext* previous;
    IFTContext () : previous(current) {
        current = this;
    }
    ~IFTContext () {
        assert(current == this);
        current = previous;
    }

    std::vector<SwizzleOp> swizzle_ops;
    std::vector<InitOp> init_ops;
    void do_swizzles ();
    void do_inits ();
};
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
void ser_collect_key_string (StrVector&, const String&);
void ser_collect_keys (const Traversal&, StrVector&);

 // Implement set_keys by removing keys from a std::vector<Str>
bool ser_claim_key (std::vector<Str>&, Str);
void ser_claim_keys (const Traversal&, std::vector<Str>&, bool optional);
void ser_set_keys (const Traversal&, std::vector<Str>&&);

 // If the attr isn't found, returns false and doesn't call the callback
bool ser_maybe_attr (const Traversal&, Str, AccessMode, TravCallback);
 // Throws if the attr isn't found
void ser_attr (const Traversal&, Str, AccessMode, TravCallback);

 ///// Elem operations
usize ser_get_length (const Traversal&);
 // Implement set_length by counting up used length
void ser_claim_length (const Traversal&, usize& claimed, usize len);
void ser_set_length (const Traversal&, usize);

 // If elem is out of range, returns false and doesn't call the callback
bool ser_maybe_elem (const Traversal&, usize, AccessMode, TravCallback);
 // Throws if elem is out of bounds
void ser_elem (const Traversal&, usize, AccessMode, TravCallback);

} // namespace ayu::in
