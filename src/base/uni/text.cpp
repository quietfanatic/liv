#include "text.h"

#include <cctype>

namespace uni {

 // From what I see, different implementations of natural sort vary on their
 // behavior in corner cases.  For example:
 //     ls -v      |   nemo
 //   "001" < "01" | "01" < "001"
 //   "ab" < "a "  | "a " < "ab"
 // I'm going to side with nemo's behavior because it looks easier. : )
int natural_compare (Str a, Str b) {
    auto ap = a.begin();
    auto bp = b.begin();
    while (ap != a.end() && bp != b.end()) {
         // If one has a number but not the other, the number comes first
        if (std::isdigit(*ap) && !std::isdigit(*bp)) return -1;
        if (!std::isdigit(*ap) && std::isdigit(*bp)) return 1;
         // Skip but count zeroes
        usize azeros = 0;
        usize bzeros = 0;
        while (*ap == '0') {
            azeros++; ap++;
            if (ap == a.end()) break;
        }
        while (*bp == '0') {
            bzeros++; bp++;
            if (bp == b.end()) break;
        }
         // Capture run of digits
        auto anumstart = ap;
        auto bnumstart = bp;
        while (ap != a.end() && std::isdigit(*ap)) ap++;
        while (bp != b.end() && std::isdigit(*bp)) bp++;
        auto anumlen = ap - anumstart;
        auto bnumlen = bp - bnumstart;
         // If there are more digits (after zeros), it comes after
        if (anumlen != bnumlen) {
            return anumlen < bnumlen ? -1 : 1;
        }
         // Otherwise, compare digits in the number
        for (isize i = 0; i < anumlen; i++) {
            if (anumstart[i] != bnumstart[i]) {
                return (anumstart[i] > bnumstart[i])
                     - (anumstart[i] < bnumstart[i]);
            }
        }
         // Digits are the same, so if there are more zeros put it after
        if (azeros != bzeros) return azeros < bzeros ? -1 : 1;
         // Zeros and digits are the same so just compare a non-digit character
        if (ap != a.end() && bp != b.end()) {
            if (*ap != *bp) return *ap < *bp ? -1 : 1;
            ap++; bp++;
        }
    }
     // Ran out of one side, so whichever has more left comes after
    return a.end() - ap < b.end() - bp ? -1 : 1;
}

} using namespace uni;

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"

static tap::TestSet tests ("base/uni/text", []{
    using namespace tap;
    is(natural_compare("a", "b"), -1);
    is(natural_compare("3", "2"), 1);
    is(natural_compare("a1b", "a10b"), -1);
    is(natural_compare("a9b", "a10b"), -1);
    is(natural_compare("a9b", "ab"), -1);
    is(natural_compare("a1b", "a01b"), -1);
    is(natural_compare("a0", "a "), -1);
    is(natural_compare("a b", "ab"), -1);
    is(natural_compare("01", "001"), -1);
    done_testing();
});
#endif
