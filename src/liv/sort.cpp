#include "sort.h"

#include <algorithm>
#include <random>
#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/traversal/from-tree.h"
#include "../dirt/ayu/traversal/to-tree.h"
#include "../dirt/iri/path.h"
#include "../dirt/uni/text.h"

namespace liv {

void do_sort (IRI* begin, IRI* end, SortMethod method) {
    switch (method.criterion) {
        case SortCriterion::Natural: {
            if (!!(method.flags & SortFlags::Reverse)) {
                std::stable_sort(begin, end, [](const IRI& a, const IRI& b){
                    return uni::natural_lessthan(b.path(), a.path());
                });
            }
            else {
                std::stable_sort(begin, end, [](const IRI& a, const IRI& b){
                    return uni::natural_lessthan(a.path(), b.path());
                });
            }
            break;
        }
        case SortCriterion::Unicode: {
            if (!!(method.flags & SortFlags::Reverse)) {
                std::stable_sort(begin, end, [](const IRI& a, const IRI& b){
                    return b.path() < a.path();
                });
            }
            else {
                std::stable_sort(begin, end, [](const IRI& a, const IRI& b){
                    return a.path() < b.path();
                });
            }
            break;
        }
        case SortCriterion::LastModified: {
             // Getting file mod times is slow, especially with the
             // std::filesystem library, so cache the results
            using Time = decltype(fs::last_write_time(fs::path()));
            auto proj = UniqueArray<std::pair<Time, IRI>>(
                end - begin, [begin](usize i){
                    return std::pair<Time, IRI>{
                        fs::last_write_time(iri::to_fs_path(begin[i])),
                        move(begin[i])
                    };
                }
            );
             // Do the sort
            if (!!(method.flags & SortFlags::Reverse)) {
                std::stable_sort(proj.begin(), proj.end(), [](const auto& a, const auto& b){
                    return b.first < a.first;
                });
            }
            else {
                std::stable_sort(proj.begin(), proj.end(), [](const auto& a, const auto& b){
                    return a.first < b.first;
                });
            }
             // Undo the projection
            proj.consume([begin, b{proj.begin()}](const auto& p){
                new (&begin[&p - b]) IRI(move(p.second));
            });
            break;
        }
        case SortCriterion::FileSize: {
            auto proj = UniqueArray<std::pair<usize, IRI>>(
                end - begin, [begin](usize i){
                    return std::pair<usize, IRI>{
                        fs::file_size(iri::to_fs_path(begin[i])),
                        move(begin[i])
                    };
                }
            );
            if (!!(method.flags & SortFlags::Reverse)) {
                std::stable_sort(proj.begin(), proj.end(), [](const auto& a, const auto& b){
                    return b.first < a.first;
                });
            }
            else {
                std::stable_sort(proj.begin(), proj.end(), [](const auto& a, const auto& b){
                    return a.first < b.first;
                });
            }
            proj.consume([begin, b{proj.begin()}](const auto& p){
                new (&begin[&p - b]) IRI(move(p.second));
            });
            break;
        }
        case SortCriterion::Shuffle: {
            static std::random_device rd;
            static std::mt19937 g (rd());
            std::shuffle(begin, end, g);
            break;
        }
        case SortCriterion::Unsorted: break;
        default: never();
    }
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
