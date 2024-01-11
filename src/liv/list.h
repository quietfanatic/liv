#pragma once

#include "../dirt/uni/arrays.h"
#include "../dirt/iri/iri.h"
#include "common.h"

namespace liv {

UniqueArray<IRI> read_list (const IRI& loc);

void write_list (const IRI& loc, Slice<IRI> entries);

void add_to_list (const IRI& list, const IRI& entry, SortMethod);

void remove_from_list (const IRI& list, const IRI& entry);

} // liv
