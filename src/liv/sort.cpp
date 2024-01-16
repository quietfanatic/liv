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
    IRI* iris, uint32 len, SortMethod method, uint32* indexes, void* props
) {
    for (usize i = 0; i < len; i++) {
        indexes[i] = i;
    }
    std::stable_sort(indexes, indexes + len, [=](uint32 a, uint32 b){
        if (!!(method.flags & SortFlags::Reverse)) {
            auto t = a; a = b; b = t;
        }
        switch (method.criterion) {
            case SortCriterion::Natural: {
                return uni::natural_lessthan(iris[a].path(), iris[b].path());
            }
            case SortCriterion::Unicode: {
                 // Make sure we put UTF-8 high bytes after ASCII bytes.  If we
                 // have to go this far, we should consider making strings hold
                 // char8_t by default instead of char...
                return GenericStr<char8_t>(iris[a].path()) <
                       GenericStr<char8_t>(iris[b].path());
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
     // looks wild but it works and is O(n).  Basically, we're finding closed
     // loops of indexes, and rotating the items backwards along that loop.
    for (uint32 i = 0; i < len; i++) {
         // Don't need to do this one or already did it.
        if (indexes[i] == i) continue;
         // Start the loop by picking up the first item.  We won't put it back
         // down until we reach the end of the loop.
        IRI tmp;
        std::memcpy((void*)&tmp, &iris[i], sizeof(IRI));
        for (;;) {
            uint32 next = indexes[i];
             // Move the item from the next slot into this slot.
            std::memcpy((void*)&iris[i], &iris[next], sizeof(IRI));
             // This slot is now correct
            indexes[i] = i;
             // Now switch to the next slot
            i = next;
            next = indexes[i];
             // Stop if we're on the last slot (by checking if the next slot has
             // already been solved).
            if (indexes[next] == next) break;
        }
         // Finally drop the first item into the last slot
        std::memcpy((void*)&iris[i], &tmp, sizeof(IRI));
        std::memset((void*)&tmp, 0, sizeof(IRI));
         // Mark the slot as done
        uint32 next = indexes[i];
        indexes[i] = i;
         // Return to the beginning
        i = next;
    }
}

NOINLINE static
void sort_by_modtime (
    IRI* iris, uint32 len, SortMethod method, uint32* indexes, ModTime* modtimes
) {
    for (uint32 i = 0; i < len; i++) {
        modtimes[i] = fs::last_write_time(iri::to_fs_path(iris[i]));
    }
    sort_with_buffers(iris, len, method, indexes, modtimes);
}

NOINLINE static
void sort_by_size (
    IRI* iris, uint32 len, SortMethod method, uint32* indexes, usize* sizes
) {
    for (uint32 i = 0; i < len; i++) {
        sizes[i] = fs::file_size(iri::to_fs_path(iris[i]));
    }
    sort_with_buffers(iris, len, method, indexes, sizes);
}

static
void sort_by_path_on_stack (IRI* iris, uint32 len, SortMethod method) {
    uint32 indexes [len];
    sort_with_buffers(iris, len, method, indexes, null);
}

static
void sort_by_path_on_heap (IRI* iris, uint32 len, SortMethod method) {
    auto indexes = std::unique_ptr<uint32[]>(new uint32[len]);
    sort_with_buffers(iris, len, method, &indexes[0], null);
}

static
void sort_by_modtime_on_stack (IRI* iris, uint32 len, SortMethod method) {
    uint32 indexes [len];
    ModTime modtimes [len];
    sort_by_modtime(iris, len, method, indexes, modtimes);
}

static
void sort_by_modtime_on_heap (IRI* iris, uint32 len, SortMethod method) {
    auto indexes = std::unique_ptr<uint32[]>(new uint32[len]);
    auto modtimes = std::unique_ptr<ModTime[]>(new ModTime[len]);
    sort_by_modtime(iris, len, method, &indexes[0], &modtimes[0]);
}

static
void sort_by_size_on_stack (IRI* iris, uint32 len, SortMethod method) {
    uint32 indexes [len];
    usize sizes [len];
    sort_by_size(iris, len, method, indexes, sizes);
}

static
void sort_by_size_on_heap (IRI* iris, uint32 len, SortMethod method) {
    auto indexes = std::unique_ptr<uint32[]>(new uint32[len]);
    auto sizes = std::unique_ptr<usize[]>(new usize[len]);
    sort_by_size(iris, len, method, &indexes[0], &sizes[0]);
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
    usize len = end - begin;
    switch (method.criterion) {
        case SortCriterion::Natural:
        case SortCriterion::Unicode: {
            auto bytes = len * sizeof(uint32);
            if (bytes <= stack_threshold) {
                sort_by_path_on_stack(begin, len, method);
            }
            else {
                sort_by_path_on_heap(begin, len, method);
            }
            break;
        }
        case SortCriterion::LastModified: {
            auto bytes = len * (sizeof(uint32) + sizeof(ModTime));
            if (bytes <= stack_threshold) {
                sort_by_modtime_on_stack(begin, len, method);
            }
            else {
                sort_by_modtime_on_heap(begin, len, method);
            }
            break;
        }
        case SortCriterion::FileSize: {
            auto bytes = len * (sizeof(uint32) + sizeof(usize));
            if (bytes <= stack_threshold) {
                sort_by_size_on_stack(begin, len, method);
            }
            else {
                sort_by_size_on_heap(begin, len, method);
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
        raise(e_General, "No sort criterion in sort method");
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

#ifndef TAP_DISABLE_TESTS
#include "../dirt/tap/tap.h"

static tap::TestSet tests ("liv/sort", []{
    using namespace tap;

     // Deterministic seed for performance testing
    std::mt19937 gen (0);
    std::uniform_int_distribution dist(0, 99999);

    IRI base ("file:/");

    auto iris = UniqueArray<IRI>(4000, [&](usize){
        return IRI(cat(dist(gen)), base);
    });

    sort_iris(
        iris.begin(), iris.end(),
        SortMethod{SortCriterion::Natural, SortFlags{}}
    );
    bool sorted = true;
    for (usize i = 0; i < iris.size() - 1; i++) {
        sorted &= !uni::natural_lessthan(iris[i+1].path(), iris[i].path());
    }
    ok(sorted);

    done_testing();
});
#endif
