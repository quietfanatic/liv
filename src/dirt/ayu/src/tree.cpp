#include "../tree.h"

#include "../describe.h"

namespace ayu {
namespace in {

NOINLINE
void delete_Tree_data (TreeRef t) {
     // Delete by manifesting an array and letting its destructor run.
     // We're using Unique* instead of Shared* because we've already run down
     // the reference count.
    switch (t->rep) {
        case REP_SHAREDSTRING: {
            UniqueString::Materialize(
                (char*)t->data.as_char_ptr, t->length
            );
            break;
        }
        case REP_ARRAY: {
            UniqueArray<Tree>::Materialize(
                (Tree*)t->data.as_array_ptr, t->length
            );
            break;
        }
        case REP_OBJECT: {
            UniqueArray<TreePair>::Materialize(
                (TreePair*)t->data.as_object_ptr, t->length
            );
            break;
        }
        case REP_ERROR: {
            UniqueArray<std::exception_ptr>::Materialize(
                (std::exception_ptr*)t->data.as_error_ptr, t->length
            );
            break;
        }
        default: never();
    }
}

[[noreturn]]
void bad_Tree_form (TreeRef t, TreeForm form) {
    if (t->rep == REP_ERROR) std::rethrow_exception(std::exception_ptr(*t));
    else if (t->form == form) never();
    else throw X<WrongForm>(form, t);
}

} using namespace in;

bool operator == (TreeRef a, TreeRef b) {
    if (a->rep != b->rep) {
         // Special case int/float comparisons
        if (a->rep == REP_INT64 && b->rep == REP_DOUBLE) {
            return int64(*a) == double(*b);
        }
        else if (a->rep == REP_DOUBLE && b->rep == REP_INT64) {
            return double(*a) == int64(*b);
        }
         // Comparison between different-lifetime strings
        else if ((a->rep == REP_STATICSTRING && b->rep == REP_SHAREDSTRING)
              || (a->rep == REP_SHAREDSTRING && b->rep == REP_STATICSTRING)
        ) {
            return Str(*a) == Str(*b);
        }
         // Otherwise different reps = different values.
        return false;
    }
    else switch (a->rep) {
        case REP_NULL: return true;
        case REP_BOOL: return bool(*a) == bool(*b);
        case REP_INT64: return int64(*a) == int64(*b);
        case REP_DOUBLE: {
            auto av = double(*a);
            auto bv = double(*b);
            return av == bv || (av != av && bv != bv);
        }
        case REP_STATICSTRING:
        case REP_SHAREDSTRING: {
            return Str(*a) == Str(*b);
        }
        case REP_ARRAY: {
            return TreeArraySlice(*a) == TreeArraySlice(*b);
        }
        case REP_OBJECT: {
             // Allow attributes to be in different orders
            auto ao = TreeObjectSlice(*a);
            auto bo = TreeObjectSlice(*b);
            if (ao.size() != bo.size()) return false;
            else for (auto& ap : ao) {
                for (auto& bp : bo) {
                    if (ap.first == bp.first) {
                        if (ap.second == bp.second) break;
                        else return false;
                    }
                }
            }
            return true;
        }
        case REP_ERROR: return false;
        default: never();
    }
}

} using namespace ayu;

AYU_DESCRIBE(ayu::TreeForm,
    values(
        value("undefined", UNDEFINED),
        value("null", NULLFORM),
        value("bool", BOOL),
        value("number", NUMBER),
        value("string", STRING),
        value("array", ARRAY),
        value("object", OBJECT),
        value("error", ERROR)
    )
)

 // Theoretically we could add support for attr and elem access to this, but
 // we'll save that for when we need it.
AYU_DESCRIBE(ayu::Tree,
    to_tree([](const Tree& v){ return v; }),
    from_tree([](Tree& v, const Tree& t){ v = t; })
)

AYU_DESCRIBE(ayu::TreeError,
    delegate(base<Error>())
)

AYU_DESCRIBE(ayu::WrongForm,
    elems(
        elem(base<TreeError>(), include),
        elem(&WrongForm::form),
        elem(&WrongForm::tree)
    )
)

AYU_DESCRIBE(ayu::CantRepresent,
    elems(
        elem(base<TreeError>(), include),
        elem(&CantRepresent::type_name),
        elem(&CantRepresent::tree)
    )
)

#ifndef TAP_DISABLE_TESTS
#include "../../tap/tap.h"

static tap::TestSet tests ("dirt/ayu/tree", []{
    using namespace tap;
    isnt(Tree(null), Tree(0), "Comparisons fail on different types");
    is(Tree(3), Tree(3.0), "Compare integers with floats");
    isnt(Tree(3), Tree(3.1), "Compare integers with floats (!=)");
    is(Tree(0.0/0.0), Tree(0.0/0.0), "Tree of NAN equals Tree of NAN");
    is(Str(Tree("asdfg")), "asdfg", "Round-trip strings");
    is(Str(Tree("qwertyuiop")), "qwertyuiop", "Round-trip long strings");
    throws<WrongForm>([]{ int(Tree("0")); }, "Can't convert string to integer");
    try_is<int>([]{ return int(Tree(3.0)); }, 3, "Convert floating to integer");
    try_is<double>([]{ return double(Tree(3)); }, 3.0, "Convert integer to floating");
    throws<CantRepresent>([]{
        int(Tree(3.5));
    }, "Can't convert 3.5 to integer");
    throws<CantRepresent>([]{
        int8(Tree(1000));
    }, "Can't convert 1000 to int8");
    throws<CantRepresent>([]{
        uint8(Tree(-1));
    }, "Can't convert -1 to uint8");
    is(Tree(TreeArray{Tree(3), Tree(4)}), Tree(TreeArray{Tree(3), Tree(4)}), "Compare arrays.");
    isnt(Tree(TreeArray{Tree(3), Tree(4)}), Tree(TreeArray{Tree(4), Tree(3)}), "Compare unequal arrays.");
    is(
        Tree(TreeObject{TreePair{"a", Tree(0)}, TreePair{"b", Tree(1)}}),
        Tree(TreeObject{TreePair{"b", Tree(1)}, TreePair{"a", Tree(0)}}),
        "TreeObject with same attributes in different order are equal"
    );
    isnt(
        Tree(TreeObject{TreePair{"a", Tree(0)}, TreePair{"b", Tree(1)}}),
        Tree(TreeObject{TreePair{"b", Tree(1)}, TreePair{"a", Tree(0)}, TreePair{"c", Tree(3)}}),
        "Extra attribute in second object makes it unequal"
    );
    done_testing();
});
#endif
