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

 // Go through a bit of work to only instantiate a single copy of
 // std::stable_sort
struct Comparator {
    using F = bool (const Comparator&, uint32, uint32) noexcept;
    IRI* iris;
    void* props;
    F* f;
    bool operator() (uint32 a, uint32 b) const noexcept {
        return (*f)(*this, a, b);
    }
    template <SortMethod method>
    static bool cmp (const Comparator& self, uint32 a, uint32 b) noexcept {
        auto iris = self.iris;
        auto modtimes = (ModTime*)self.props;
        auto sizes = (usize*)self.props;
        if (!!(method.flags & SortFlags::Reverse)) {
            uint32 t = a; a = b; b = t;
        }
        switch (method.criterion) {
            case SortCriterion::Natural: {
                return uni::natural_lessthan(
                    iris[a].path(),
                    iris[b].path()
                );
            }
            case SortCriterion::Unicode: {
                 // Make sure we put UTF-8 high bytes after ASCII bytes.  If we
                 // have to go this far, we should consider making strings hold
                 // char8_t by default instead of char...
                return GenericStr<char8_t>(iris[a].path()) <
                       GenericStr<char8_t>(iris[b].path());
            }
            case SortCriterion::LastModified: {
                return modtimes[a] < modtimes[b];
            }
            case SortCriterion::FileSize: {
                return sizes[a] < sizes[b];
            }
            default: never();
        }
    }
};

NOINLINE static
void sort_with_props (
    IRI* iris, uint32 len, Comparator::F* comp, void* props
) {
    auto indexes = std::unique_ptr<uint32[]>(new uint32[len]);
    for (usize i = 0; i < len; i++) {
        indexes[i] = i;
    }
    std::stable_sort(&indexes[0], &indexes[0] + len, Comparator(iris, props, comp));
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

NOINLINE
void sort_iris (IRI* begin, IRI* end, SortMethod method) {
    plog("starting sort");
    usize len = end - begin;
    using enum SortCriterion;
    using enum SortFlags;
    switch (method.criterion) {
        case Natural: {
            auto comp = !(method.flags & Reverse)
                ? &Comparator::cmp<SortMethod{Natural, SortFlags{}}>
                : &Comparator::cmp<SortMethod{Natural, Reverse}>;

            sort_with_props(begin, len, comp, null);
            break;
        }
        case Unicode: {
            auto comp = !(method.flags & Reverse)
                ? &Comparator::cmp<SortMethod{Unicode, SortFlags{}}>
                : &Comparator::cmp<SortMethod{Unicode, Reverse}>;

            sort_with_props(begin, len, comp, null);
            break;
        }
        case LastModified: {
            auto comp = !(method.flags & Reverse)
                ? &Comparator::cmp<SortMethod{LastModified, SortFlags{}}>
                : &Comparator::cmp<SortMethod{LastModified, Reverse}>;

            auto modtimes = std::unique_ptr<ModTime[]>(new ModTime[len]);
            for (uint32 i = 0; i < len; i++) {
                modtimes[i] = fs::last_write_time(iri::to_fs_path(begin[i]));
            }
            sort_with_props(begin, len, comp, &modtimes[0]);
            break;
        }
        case FileSize: {
            auto comp = !(method.flags & Reverse)
                ? &Comparator::cmp<SortMethod{FileSize, SortFlags{}}>
                : &Comparator::cmp<SortMethod{FileSize, Reverse}>;

            auto sizes = std::unique_ptr<usize[]>(new usize[len]);
            for (uint32 i = 0; i < len; i++) {
                sizes[i] = fs::file_size(iri::to_fs_path(begin[i]));
            }
            sort_with_props(begin, len, comp, &sizes[0]);
            break;
        }
        case Shuffle: {
            static std::random_device rd;
            static std::mt19937 g (rd());
            std::shuffle(begin, end, g);
            break;
        }
        case Unsorted: break;
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
