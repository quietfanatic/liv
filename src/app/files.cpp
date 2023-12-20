#include "files.h"

#include <filesystem>
#include <unordered_set>
#include "../dirt/uni/io.h"
#include "../dirt/uni/text.h"
#include "settings.h"

namespace app {

AnyString containing_folder (Str filename) {
    return AnyString(fs::path(filename).remove_filename().u8string());
}

UniqueArray<AnyString> expand_folder (
    const Settings* settings, Str foldername
) {
    auto& extensions = settings->get(&FilesSettings::supported_extensions);

    UniqueArray<AnyString> r;
    for (auto& entry : fs::directory_iterator(foldername)) {
        auto name = AnyString(entry.path().u8string());
        usize i;
        for (i = name.size(); i > 0; --i) {
            if (name[i-1] == '.') break;
        }
        Str ext = i ? name.substr(i) : "";
        if (!extensions.count(StaticString(ext))) continue;
        r.emplace_back(move(name));
    }
    std::sort(
        r.begin(), r.end(), &uni::natural_lessthan
    );
    return r;
}

UniqueArray<AnyString> expand_recursively (
    const Settings* settings, Slice<AnyString> filenames
) {
    auto& extensions = settings->get(&FilesSettings::supported_extensions);

    UniqueArray<AnyString> r;
    for (auto& given : filenames) {
        if (fs::is_directory(given)) {
            usize subfiles_begin = r.size();
            for (auto& entry : fs::recursive_directory_iterator(given)) {
                auto name = AnyString(entry.path().u8string());
                usize i;
                for (i = name.size(); i > 0; --i) {
                    if (name[i-1] == '.') break;
                }
                Str ext = i ? name.substr(i) : "";
                if (!extensions.count(StaticString(ext))) continue;
                r.emplace_back(move(name));
            }
            std::sort(
                r.begin() + subfiles_begin, r.end(), &uni::natural_lessthan
            );
        }
        else {
             // Don't check the file extension for explicitly specified
             // files.
            r.emplace_back(given);
        }
    }
    return r;
}

UniqueArray<AnyString> read_list (Str list_filename) {
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
    }
    if (lines.back() == "") lines.pop_back();
    return lines;
}

} // namespace app
