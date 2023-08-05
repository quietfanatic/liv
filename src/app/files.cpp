#include "files.h"

#include <filesystem>
#include <unordered_set>
#include "../dirt/uni/io.h"
#include "../dirt/uni/text.h"
#include "settings.h"

namespace app {

FilesToOpen expand_files (
    const Settings* settings, UniqueArray<AnyString>&& specified
) {
    auto& extensions = settings->get(&FilesSettings::supported_extensions);

    if (specified.size() == 1 &&
        fs::exists(specified[0]) &&
        !fs::is_directory(specified[0])
    ) {
         // Only one file was specified, and it isn't a folder, so include
         // everything else in the same folder as the file (but not recursively)
        auto folder_p = fs::path(specified[0]).remove_filename();
        FilesToOpen r;
        r.folder = AnyString(folder_p.u8string());
        for (auto& entry : fs::directory_iterator(folder_p)) {
            auto name = AnyString(entry.path().u8string());
            usize i;
            for (i = name.size(); i > 0; --i) {
                if (name[i-1] == '.') break;
            }
            Str ext = i ? name.substr(i) : "";
            if (!extensions.count(StaticString(ext))) continue;
            r.files.emplace_back(move(name));
        }
        std::sort(
            r.files.begin(), r.files.end(), &uni::natural_lessthan
        );
         // Start the book open to the actual file specified.
        r.start_index = usize(-1);
        for (auto& file : r.files) {
            if (file == specified[0]) {
                r.start_index = &file - r.files.begin();
                break;
            }
        }
         // If the specified file is not in the list, then it must have been
         // deleted or moved between fs::exists and fs::directory_iterator.
         // There's nothing reasonable to do in this case, so just crash.
        require(r.start_index != usize(-1));
        return r;
    }
    else {
         // Multiple files were specified or the specified file was a directory,
         // so search through all specified directories recursively.
        FilesToOpen r;
        for (auto& file : specified) {
            if (fs::is_directory(file)) {
                usize subfiles_begin = r.files.size();
                for (auto& entry : fs::recursive_directory_iterator(file)) {
                    auto name = AnyString(entry.path().u8string());
                    usize i;
                    for (i = name.size(); i > 0; --i) {
                        if (name[i-1] == '.') break;
                    }
                    Str ext = i ? name.substr(i) : "";
                    if (!extensions.count(StaticString(ext))) continue;
                    r.files.emplace_back(move(name));
                }
                std::sort(
                    r.files.begin() + subfiles_begin,
                    r.files.end(),
                    &uni::natural_lessthan
                );
            }
            else {
                 // Don't check the file extension for explicitly specified
                 // files.
                r.files.emplace_back(move(file));
            }
        }
        return r;
    }
}

UniqueArray<AnyString> read_list (AnyString list_filename) {
    UniqueArray<AnyString> lines {""};
    if (list_filename == "-") {
         // TODO: Make ayu support stdin for string_from_file.
        for (;;) {
            int c = getchar();
            if (c == EOF) break;
            else if (c == '\n') {
                if (lines.back() != "") {
                    lines.emplace_back();
                }
            }
            else lines.back().push_back(c);
        }
    }
    else {
        for (char c : ayu::string_from_file(list_filename)) {
            if (c == '\n') {
                if (lines.back() != "") {
                    lines.emplace_back();
                }
            }
            else lines.back().push_back(c);
        }
         // Set working directory to folder containing list, to makes list's
         // filenames relative to itself.
        fs::current_path(fs::absolute(list_filename).remove_filename());
    }
    if (lines.back() == "") lines.pop_back();
    return lines;
}

} // namespace app
