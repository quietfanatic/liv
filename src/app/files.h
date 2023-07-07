#pragma once

#include "../dirt/uni/common.h"
#include "../dirt/uni/strings.h"
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
