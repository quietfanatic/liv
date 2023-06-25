#pragma once

#include <vector>
#include "../base/uni/common.h"
#include "../base/uni/strings.h"
#include "common.h"

namespace app {

struct FilesToOpen {
    UniqueArray<AnyString> files;
    AnyString folder = "";
    usize start_index = 0;
};

FilesToOpen expand_files (const Settings*, UniqueArray<AnyString>&& filenames);
 // Note: This will set the working directory to the folder containing
 // list_filename.
UniqueArray<AnyString> read_list (AnyString list_filename);

} // namespace files
