#pragma once

#include <vector>
#include "../base/uni/common.h"
#include "common.h"

namespace app {

struct FilesToOpen {
    std::vector<String> files;
    String folder = ""s;
    usize start_index = 0;
};

FilesToOpen expand_files (const Settings*, std::vector<String>&& filenames);
 // Note: This will set the working directory to the folder containing
 // list_filename.
std::vector<String> read_list (Str list_filename);

} // namespace files
