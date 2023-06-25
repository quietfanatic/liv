#include "files.h"

#include <filesystem>
#include <unordered_set>
#include "../base/uni/text.h"
#include "settings.h"

namespace app {

FilesToOpen expand_files (
    const Settings* settings, std::vector<std::string>&& specified
) {
    auto& extensions = settings->get(&FilesSettings::supported_extensions);

    if (size(specified) == 1 &&
        fs::exists(specified[0]) &&
        !fs::is_directory(specified[0])
    ) {
         // Only one file was specified, and it isn't a folder, so include
         // everything else in the same folder as the file (but not recursively)
        auto folder_p = fs::path(specified[0]).remove_filename();
        FilesToOpen r;
         // std::u8string was a terrible idea
        r.folder = reinterpret_cast<std::string&&>(move(folder_p.u8string()));
        for (auto& entry : fs::directory_iterator(folder_p)) {
            std::u8string u8name = entry.path().u8string();
            std::string& name = reinterpret_cast<std::string&>(u8name);
            OldStr extension;
            usize dotpos = name.rfind('.');
            if (dotpos != std::string::npos) {
                extension = OldStr(&name[dotpos+1], size(name) - dotpos - 1);
            }
            if (!extensions.count(std::string(extension))) continue;
            r.files.emplace_back(move(name));
        }
        std::sort(
            r.files.begin(), r.files.end(), &uni::natural_lessthan
        );
         // Start the book open to the actual file specified.
        r.start_index = usize(-1);
        for (usize i = 0; i < size(r.files); i++) {
            if (r.files[i] == specified[0]) {
                r.start_index = i;
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
                usize subfiles_begin = size(r.files);
                for (auto& entry : fs::recursive_directory_iterator(file)) {
                    std::u8string u8name = entry.path().u8string();
                    std::string& name = reinterpret_cast<std::string&>(u8name);
                    OldStr extension;
                    usize dotpos = name.rfind('.');
                    if (dotpos != std::string::npos) {
                        extension = OldStr(&name[dotpos+1], size(name) - dotpos - 1);
                    }
                    if (!extensions.count(std::string(extension))) continue;
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

std::vector<std::string> read_list (Str list_filename) {
    std::vector<std::string> lines {""};
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
            else lines.back() += char(c);
        }
    }
    else {
        for (char c : ayu::string_from_file(Str(list_filename))) {
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
