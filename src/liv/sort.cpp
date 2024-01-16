#include "sort.h"

#include <algorithm>
#include <random>
#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/traversal/from-tree.h"
#include "../dirt/ayu/traversal/to-tree.h"
#include "../dirt/iri/path.h"
#include "../dirt/uni/text.h"
#include "common.h"

namespace liv {

using ModTime = decltype(fs::last_write_time(fs::path()));

 // All of these need to be in the same function so we can instantiate as few
 // copies of std::stable_sort as possible.
NOINLINE static
void sort_with_buffers (
    IRI* begin, IRI* end, SortMethod method, uint32* indexes, void* props
) {
    auto len = uint32(end - begin);
    for (usize i = 0; i < len; i++) {
        indexes[i] = i;
    }
    std::stable_sort(indexes, indexes + len, [=](uint32 a, uint32 b){
        if (!!(method.flags & SortFlags::Reverse)) {
            auto t = a; a = b; b = t;
        }
        switch (method.criterion) {
            case SortCriterion::Natural: {
                return uni::natural_lessthan(begin[a].path(), begin[b].path());
            }
            case SortCriterion::Unicode: {
                 // Make sure we put UTF-8 high bytes after ASCII bytes.  If we
                 // have to go this far, we should consider making strings hold
                 // char8_t by default instead of char...
                return GenericStr<char8_t>(begin[a].path()) <
                       GenericStr<char8_t>(begin[b].path());
            }
            case SortCriterion::LastModified: {
                auto modtimes = (ModTime*)props;
                return modtimes[a] < modtimes[b];
            }
            case SortCriterion::FileSize: {
                auto sizes = (usize*)props;
                return sizes[a] < sizes[b];
            }
            default: never();
        }
    });
     // Now reorder the input according to the sorted indexes.  This algorithm
     // looks wild but it works and is O(n).
    for (uint32 i = 0; i < len; i++) {
        using std::swap;
         // Follow a closed loop of indexes.  For a loop of N items, we actually
         // have to do N-1 swaps, so stop following the loop just before we
         // would finish it.
        while (indexes[indexes[i]] != indexes[i]) {
            swap(begin[i], begin[indexes[i]]);
            swap(indexes[i], i);
        }
         // Finish the loop but don't do the Nth swap.
        swap(indexes[i], i);
    }
}

NOINLINE static
void sort_by_modtime (
    IRI* begin, IRI* end, SortMethod method, uint32* indexes, ModTime* modtimes
) {
    for (usize i = 0; i < uint32(end - begin); i++) {
        modtimes[i] = fs::last_write_time(iri::to_fs_path(begin[i]));
    }
    sort_with_buffers(begin, end, method, indexes, modtimes);
}

NOINLINE static
void sort_by_size (
    IRI* begin, IRI* end, SortMethod method, uint32* indexes, usize* sizes
) {
    for (usize i = 0; i < uint32(end - begin); i++) {
        sizes[i] = fs::file_size(iri::to_fs_path(begin[i]));
    }
    sort_with_buffers(begin, end, method, indexes, sizes);
}

NOINLINE static
void sort_by_path_on_stack (IRI* begin, IRI* end, SortMethod method) {
    uint32 indexes [end - begin];
    sort_with_buffers(begin, end, method, indexes, null);
}

NOINLINE static
void sort_by_path_on_heap (IRI* begin, IRI* end, SortMethod method) {
    auto indexes = std::unique_ptr<uint32[]>(new uint32[end - begin]);
    sort_with_buffers(begin, end, method, &indexes[0], null);
}

NOINLINE static
void sort_by_modtime_on_stack (IRI* begin, IRI* end, SortMethod method) {
    uint32 indexes [end - begin];
    ModTime modtimes [end - begin];
    sort_by_modtime(begin, end, method, indexes, modtimes);
}

NOINLINE static
void sort_by_modtime_on_heap (IRI* begin, IRI* end, SortMethod method) {
    auto indexes = std::unique_ptr<uint32[]>(new uint32[end - begin]);
    auto modtimes = std::unique_ptr<ModTime[]>(new ModTime[end - begin]);
    sort_by_modtime(begin, end, method, &indexes[0], &modtimes[0]);
}

NOINLINE static
void sort_by_size_on_stack (IRI* begin, IRI* end, SortMethod method) {
    uint32 indexes [end - begin];
    usize sizes [end - begin];
    sort_by_size(begin, end, method, indexes, sizes);
}

NOINLINE static
void sort_by_size_on_heap (IRI* begin, IRI* end, SortMethod method) {
    auto indexes = std::unique_ptr<uint32[]>(new uint32[end - begin]);
    auto sizes = std::unique_ptr<usize[]>(new usize[end - begin]);
    sort_by_size(begin, end, method, &indexes[0], &sizes[0]);
}

static constexpr usize stack_threshold =
#ifdef __linux__
    1024*1024
#else
    128*1024
#endif
;

NOINLINE
void sort_iris (IRI* begin, IRI* end, SortMethod method) {
    plog("starting sort");
    switch (method.criterion) {
        case SortCriterion::Natural:
        case SortCriterion::Unicode: {
            auto bytes = (end - begin) * sizeof(uint32);
            if (bytes <= stack_threshold) {
                sort_by_path_on_stack(begin, end, method);
            }
            else {
                sort_by_path_on_heap(begin, end, method);
            }
            break;
        }
        case SortCriterion::LastModified: {
            auto bytes = (end - begin) * (sizeof(uint32) + sizeof(ModTime));
            if (bytes <= stack_threshold) {
                sort_by_modtime_on_stack(begin, end, method);
            }
            else {
                sort_by_modtime_on_heap(begin, end, method);
            }
            break;
        }
        case SortCriterion::FileSize: {
            auto bytes = (end - begin) * (sizeof(uint32) + sizeof(usize));
            if (bytes <= stack_threshold) {
                sort_by_size_on_stack(begin, end, method);
            }
            else {
                sort_by_size_on_heap(begin, end, method);
            }
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
    plog("sorted");
}

struct SortMethodToken : SortMethod { };
bool operator== (SortMethodToken a, SortMethodToken b) {
    return a.criterion == b.criterion && a.flags == b.flags;
}

ayu::Tree SortMethod_to_tree (const SortMethod& v) {
    using namespace ayu;
    auto cap = 1 + std::popcount(uint8(v.flags));
    auto a = UniqueArray<Tree>(Capacity(cap));
    SortMethodToken c = {v.criterion, SortFlags{}};
    a.push_back_expect_capacity(item_to_tree(&c));
    for (
        auto flag = SortFlags::Reverse;
        flag <= SortFlags::NotLists;
        flag <<= 1
    ) {
        if (!!(v.flags & flag)) {
            SortMethodToken f = {SortCriterion{}, flag};
            a.push_back_expect_capacity(item_to_tree(&f));
        }
    }
    return Tree(move(a));
}

void SortMethod_from_tree (SortMethod& v, const ayu::Tree& t) {
    using namespace ayu;
    v = {};
    for (auto& e : Slice<Tree>(t)) {
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
