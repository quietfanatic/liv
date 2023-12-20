#pragma once

#include "../dirt/uni/common.h"
#include "../dirt/uni/strings.h"
#include "common.h"

namespace app {

AnyString containing_folder (Str filename);

UniqueArray<AnyString> expand_folder (
    const Settings*, Str foldername
);

UniqueArray<AnyString> expand_recursively (
    const Settings*, Slice<AnyString> filenames
);

UniqueArray<AnyString> read_list (Str list_filename);

} // namespace files
