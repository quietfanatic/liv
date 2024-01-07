#pragma once

#include "../dirt/uni/arrays.h"
#include "../dirt/iri/iri.h"
#include "common.h"

namespace liv {

UniqueArray<IRI> read_list (const IRI& loc);

void write_list (const IRI& loc, Slice<IRI> entries);

} // liv