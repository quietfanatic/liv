#include "sort.h"

#include <random>
#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/traversal/from-tree.h"
#include "../dirt/ayu/traversal/to-tree.h"
#include "../dirt/iri/path.h"
#include "../dirt/uni/text.h"

namespace liv {

void do_sort (IRI* begin, IRI* end, SortMethod method) {
    if (method.criterion == SortCriterion::Shuffle) {
        static std::random_device rd;
        static std::mt19937 g (rd());
        std::shuffle(begin, end, g);
        return;
    }
    std::stable_sort(begin, end, [method](const IRI& aa, const IRI& bb){
        const IRI& a = !!(method.flags & SortFlags::Reverse)
            ? bb : aa;
        const IRI& b = !!(method.flags & SortFlags::Reverse)
            ? aa : bb;
        switch (method.criterion) {
            case SortCriterion::Natural: {
                return uni::natural_lessthan(a.path(), b.path());
            }
            case SortCriterion::Unicode: {
                return a.path() < b.path();
            }
            case SortCriterion::LastModified: {
                auto atime = fs::last_write_time(iri::to_fs_path(a));
                auto btime = fs::last_write_time(iri::to_fs_path(b));
                if (atime != btime) return atime < btime;
                else return uni::natural_lessthan(a.path(), b.path());
            }
            case SortCriterion::FileSize: {
                auto asize = fs::file_size(iri::to_fs_path(a));
                auto bsize = fs::file_size(iri::to_fs_path(b));
                if (asize != bsize) return asize < bsize;
                else return uni::natural_lessthan(a.path(), b.path());
            }
            case SortCriterion::Unsorted: {
                return false;
            }
            default: never();
        }
    });
}

struct SortMethodToken : SortMethod { };
bool operator== (SortMethodToken a, SortMethodToken b) {
    return a.criterion == b.criterion && a.flags == b.flags;
}

ayu::Tree SortMethod_to_tree (const SortMethod& v) {
    using namespace ayu;
    TreeArray a;
    SortMethodToken c = {v.criterion, SortFlags{}};
    a.push_back(item_to_tree(&c));
    for (
        auto flag = SortFlags::Reverse;
        flag <= SortFlags::NotLists;
        flag <<= 1
    ) {
        if (!!(v.flags & flag)) {
            SortMethodToken f = {SortCriterion{}, flag};
            a.push_back(item_to_tree(&f));
        }
    }
    return Tree(move(a));
}

void SortMethod_from_tree (SortMethod& v, const ayu::Tree& t) {
    using namespace ayu;
    v = {};
    for (auto e : TreeArraySlice(t)) {
        SortMethodToken token;
        item_from_tree(&token, e);
        if (token.criterion != SortCriterion{}) {
            if (v.criterion != SortCriterion{}) {
                raise(e_General, "Too many sort criteria in sort method.");
            }
            v.criterion = token.criterion;
        }
        else {
            if (!!(v.flags & token.flags)) {
                raise(e_General, "Duplicate sort flag in sort method.");
            }
            v.flags |= token.flags;
        }
    }
    if (v.criterion == SortCriterion{}) {
        raise(e_General, "No sort croteria in sort method");
    }
}

} using namespace liv;

AYU_DESCRIBE(liv::SortMethodToken,
    values(
        value("natural", {SortCriterion::Natural, SortFlags{}}),
        value("unicode", {SortCriterion::Unicode, SortFlags{}}),
        value("last_modified", {SortCriterion::LastModified, SortFlags{}}),
        value("file_size", {SortCriterion::FileSize, SortFlags{}}),
        value("shuffle", {SortCriterion::Shuffle, SortFlags{}}),
        value("unsorted", {SortCriterion::Unsorted, SortFlags{}}),
        value("reverse", {SortCriterion{}, SortFlags::Reverse}),
        value("not_args", {SortCriterion{}, SortFlags::NotArgs}),
        value("not_lists", {SortCriterion{}, SortFlags::NotLists})
    )
)

AYU_DESCRIBE(liv::SortMethod,
    to_tree(&SortMethod_to_tree),
    from_tree(&SortMethod_from_tree)
)
