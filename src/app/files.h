#pragma once

#include <vector>
#include "../base/uni/common.h"
#include "../base/uni/strings.h"
#include "common.h"

namespace app {

struct FilesToOpen {
    std::vector<std::string> files;
    std::string folder = ""s;
    usize start_index = 0;
};

FilesToOpen expand_files (const Settings*, std::vector<std::string>&& filenames);
 // Note: This will set the working directory to the folder containing
 // list_filename.
std::vector<std::string> read_list (Str list_filename);

} // namespace files
