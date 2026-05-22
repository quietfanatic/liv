#include "sort.h"

#include <algorithm>
#include <random>
#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/traversal/from-tree.h"
#include "../dirt/ayu/traversal/to-tree.h"
#include "../dirt/iri/path.h"
#include "../dirt/geo/scalar.h"
#include "../dirt/uni/text.h"
#include "common.h"

namespace liv {

using ModTime = decltype(fs::last_write_time(fs::path()));
using C = SortCriterion;
using F = SortFlags;

IRI* comparing_iris;
void* comparing_props;

 // TODO: use qsort_r or maybe SDL_qsort_r
template <SortMethod method>
int compare_iris (const void* ap, const void* bp) noexcept {
    auto iris = comparing_iris;
    auto modtimes = (ModTime*)comparing_props;
    auto sizes = (usize*)comparing_props;
    u32 a = *(u32*)ap;
    u32 b = *(u32*)bp;
    int res;
    switch (method.criterion) {
        case C::Natural: {
            assume(iris[a].has_path());
            assume(iris[b].has_path());
            res = uni::natural_compare(
                iris[a].path(),
                iris[b].path()
            );
            break;
        }
        case C::Unicode: {
            assume(iris[a].has_path());
            assume(iris[b].has_path());
             // Make sure we put UTF-8 high bytes after ASCII bytes.  If we
             // have to go this far, we should consider making strings hold
             // char8_t by default instead of char...
             //
             // TODO: This doesn't decode %-sequences!  Oops!
            Str aa = iris[a].path();
            Str bb = iris[b].path();
            usize s = geo::min(aa.size(), bb.size());
            res = std::memcmp(aa.data(), bb.data(), s);
            if (!res) res = aa.size() - bb.size();
            break;
        }
        case C::LastModified: {
            res = modtimes[a] < modtimes[b] ? -1
                : modtimes[a] > modtimes[b] ? 1 : 0;
            break;
        }
        case C::FileSize: {
            res = sizes[a] < sizes[b] ? -1 : sizes[a] > sizes[b] ? 1 : 0;
            break;
        }
        default: never();
    }
    if constexpr (method.flags % F::Reverse) {
        res = -res;
    }
    if (!res) res = a - b;
    return res;
};

using Comparator = decltype(compare_iris<SortMethod{}>);

NOINLINE static
void sort_with_props (
    IRI* iris, u32 len, Comparator* cmp, void* props
) {
     // Sort an array of indexes as a proxy for the actual array of IRIs.  We
     // need to do this because the standard library sorting algorithm doesn't
     // give the comparing function a way to see the current indexes of the
     // items it's comparing (and you can't compare the addresses of the
     // passed-in references, because they may be in a temporary buffer or
     // already moved).  However, since moving 4-byte integers is cheaper than
     // 24-byte IRIs, this ends up being slightly faster than sorting the IRI
     // array, at least for large sets (and it's much faster than sorting an
     // array of std::pair<IRI, ModTime> or such).
    auto indexes = std::unique_ptr<u32[]>(new u32[len]);
    for (usize i = 0; i < len; i++) {
        indexes[i] = i;
    }
    comparing_iris = iris;
    comparing_props = props;
    std::qsort(&indexes[0], len, sizeof(indexes[0]), cmp);
     // Now reorder the input according to the sorted indexes.  This algorithm
     // looks wild but it works and is O(n).  Basically, we're finding closed
     // loops of indexes, and rotating the items backwards along that loop.
    for (u32 i = 0; i < len; i++) {
         // Don't need to do this one or already did it.
        if (indexes[i] == i) continue;
         // Start the loop by picking up the first item.  We won't put it back
         // down until we reach the end of the loop.
        IRI tmp;
        std::memcpy((void*)&tmp, &iris[i], sizeof(IRI));
        for (;;) {
            u32 next = indexes[i];
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
        u32 next = indexes[i];
        indexes[i] = i;
         // Return to the beginning
        i = next;
    }
}

NOINLINE
void sort_iris (IRI* begin, IRI* end, SortMethod method) {
    usize len = end - begin;
    if (len <= 1) return;
    plog("starting sort");
    switch (method.criterion) {
        case C::Natural: {
            auto cmp = method.flags % F::Reverse
                ? &compare_iris<SortMethod{C::Natural, F::Reverse}>
                : &compare_iris<SortMethod{C::Natural, F::None}>;

            sort_with_props(begin, len, cmp, null);
            break;
        }
        case C::Unicode: {
            auto cmp = method.flags % F::Reverse
                ? &compare_iris<SortMethod{C::Unicode, F::Reverse}>
                : &compare_iris<SortMethod{C::Unicode, F::None}>;

            sort_with_props(begin, len, cmp, null);
            break;
        }
        case C::LastModified: {
            auto cmp = method.flags % F::Reverse
                ? &compare_iris<SortMethod{C::LastModified, F::Reverse}>
                : &compare_iris<SortMethod{C::LastModified, F::None}>;

            auto modtimes = std::unique_ptr<ModTime[]>(new ModTime[len]);
            for (u32 i = 0; i < len; i++) {
                modtimes[i] = fs::last_write_time(iri::to_filepath(begin[i]));
            }
            sort_with_props(begin, len, cmp, &modtimes[0]);
            break;
        }
        case C::FileSize: {
            auto cmp = method.flags % F::Reverse
                ? &compare_iris<SortMethod{C::FileSize, F::Reverse}>
                : &compare_iris<SortMethod{C::FileSize, F::None}>;

            auto sizes = std::unique_ptr<usize[]>(new usize[len]);
            for (u32 i = 0; i < len; i++) {
                sizes[i] = fs::file_size(iri::to_filepath(begin[i]));
            }
            sort_with_props(begin, len, cmp, &sizes[0]);
            break;
        }
        case C::Shuffle: {
             // We don't need that high-quality of randomness.
            static std::minstd_rand gen (
                std::chrono::system_clock::now().time_since_epoch().count()
            );
            std::shuffle(begin, end, gen);
            break;
        }
        case C::Unsorted: break;
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
    auto cap = 1 + std::popcount(u8(v.flags));
    auto a = UniqueArray<Tree>(Capacity(cap));
    SortMethodToken c = {v.criterion, F::None};
    a.push_back_assume_capacity(item_to_tree(&c));
    for (
        auto flag = F::Reverse;
        flag <= F::NotLists;
        flag <<= 1
    ) {
        if (v.flags % flag) {
            SortMethodToken f = {C::None, flag};
            a.push_back_assume_capacity(item_to_tree(&f));
        }
    }
    return Tree(move(a));
}

bool SortMethod_from_tree (SortMethod& v, const ayu::Tree& t) {
    using namespace ayu;
    v = {};
    for (auto& e : Slice<Tree>(t)) {
        SortMethodToken token;
        item_from_tree(&token, e);
        if (token.criterion != C::None) {
            if (v.criterion != C::None) {
                raise(e_General, "Too many sort criteria in sort method.");
            }
            v.criterion = token.criterion;
        }
        else {
            if (v.flags % token.flags) {
                raise(e_General, "Duplicate sort flag in sort method.");
            }
            v.flags |= token.flags;
        }
    }
    if (v.criterion == C::None) {
        raise(e_General, "No sort criterion in sort method");
    }
    return true;
}

} using namespace liv;

AYU_DESCRIBE(liv::SortMethodToken,
    values(
        value("natural", {C::Natural, F::None}),
        value("unicode", {C::Unicode, F::None}),
        value("last_modified", {C::LastModified, F::None}),
        value("file_size", {C::FileSize, F::None}),
        value("shuffle", {C::Shuffle, F::None}),
        value("unsorted", {C::Unsorted, F::None}),
        value("reverse", {C::None, F::Reverse}),
        value("not_args", {C::None, F::NotArgs}),
        value("not_lists", {C::None, F::NotLists})
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

    IRI base = "file:/";

    auto iris = UniqueArray<IRI>(4000, [&](usize){
        return IRI(cat(dist(gen)), base);
    });

    sort_iris(
        iris.begin(), iris.end(),
        SortMethod{C::Natural, F::None}
    );
    bool sorted = true;
    for (usize i = 0; i < iris.size() - 1; i++) {
        sorted &= !uni::natural_lessthan(iris[i+1].path(), iris[i].path());
    }
    ok(sorted);

    done_testing();
});
#endif
