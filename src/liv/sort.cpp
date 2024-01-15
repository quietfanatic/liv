#include "sort.h"

#include <algorithm>
#include <random>
#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/traversal/from-tree.h"
#include "../dirt/ayu/traversal/to-tree.h"
#include "../dirt/iri/path.h"
#include "../dirt/uni/text.h"

namespace liv {

bool natural_lt (const IRI& a, const IRI& b) {
    return uni::natural_lessthan(a.path(), b.path());
}
bool natural_gt (const IRI& a, const IRI& b) {
    return uni::natural_lessthan(b.path(), a.path());
}
bool unicode_lt (const IRI& a, const IRI& b) {
     // Make sure we put UTF-8 bytes after ASCII bytes.  If we have to go this
     // far, we should consider making strings hold char8_t by default instead
     // of char...
    return GenericStr<char8_t>(a.path()) < GenericStr<char8_t>(b.path());
}
bool unicode_gt (const IRI& a, const IRI& b) {
    return GenericStr<char8_t>(b.path()) < GenericStr<char8_t>(a.path());
}

using Time = decltype(fs::last_write_time(fs::path()));

bool last_modified_lt (
    const std::pair<Time, IRI>& a, const std::pair<Time, IRI>& b
) {
    return a.first < b.first;
}
bool last_modified_gt (
    const std::pair<Time, IRI>& a, const std::pair<Time, IRI>& b
) {
    return a.first < b.first;
}

bool file_size_lt (
    const std::pair<usize, IRI>& a, const std::pair<usize, IRI>& b
) {
    return a.first < b.first;
}
bool file_size_gt (
    const std::pair<usize, IRI>& a, const std::pair<usize, IRI>& b
) {
    return a.first < b.first;
}

void sort_iris (IRI* begin, IRI* end, SortMethod method) {
     // TODO: Sort an integer array as a proxy.  std::sort get really bloaty if
     // you use it on multiple different element types.
    switch (method.criterion) {
        case SortCriterion::Natural: {
            std::stable_sort(begin, end, !!(method.flags & SortFlags::Reverse)
                ? natural_gt : natural_lt
            );
            break;
        }
        case SortCriterion::Unicode: {
            std::stable_sort(begin, end, !!(method.flags & SortFlags::Reverse)
                ? unicode_gt : unicode_lt
            );
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
            std::stable_sort(proj.begin(), proj.end(),
                !!(method.flags & SortFlags::Reverse)
                    ? last_modified_gt : last_modified_lt
            );
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
            std::stable_sort(proj.begin(), proj.end(),
                !!(method.flags & SortFlags::Reverse)
                    ? file_size_gt : file_size_lt
            );
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
    UniqueArray<Tree> a;
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
    for (auto e : Slice<Tree>(t)) {
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
