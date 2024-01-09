#pragma once

#include "../dirt/uni/common.h"
#include "common.h"

namespace liv {

enum class SortCriterion {
    Natural = 1,
    Unicode,
    LastModified,
    FileSize,
    Shuffle,
    Unsorted,
};

enum class SortFlags {
    Reverse = 0x1,
    NotArgs = 0x2,
    NotLists = 0x4,
     // TODO: add FoldersFirst and FoldersLast
};
DECLARE_ENUM_BITWISE_OPERATORS(SortFlags)

struct SortMethod {
    SortCriterion criterion;
    SortFlags flags;
    explicit operator bool () const { return criterion != SortCriterion{}; }
    friend bool operator== (SortMethod a, SortMethod b) = default;
};

void sort_iris (IRI* begin, IRI* end, SortMethod method);

} // liv
