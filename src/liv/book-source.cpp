#include "book-source.h"

#include "../dirt/uni/text.h"
#include "../dirt/uni/io.h"
#include "settings.h"

namespace liv {

static
AnyString containing_folder (Str path) {
    return AnyString(fs::path(path).remove_filename().u8string());
}

static
UniqueArray<AnyString> expand_folder (
    const Settings* settings, Str path
) {
    auto& extensions = settings->get(&FilesSettings::supported_extensions);

    UniqueArray<AnyString> r;
    for (auto& entry : fs::directory_iterator(path)) {
        auto child = AnyString(entry.path().u8string());
        usize i;
        for (i = child.size(); i > 0; --i) {
            if (child[i-1] == '.') break;
        }
        Str ext = i ? child.substr(i) : "";
        if (!extensions.count(StaticString(ext))) continue;
        r.emplace_back(move(child));
    }
    std::sort(
        r.begin(), r.end(), &uni::natural_lessthan
    );
    return r;
}

static
UniqueArray<AnyString> expand_recursively (
    const Settings* settings, Slice<AnyString> paths
) {
    auto& extensions = settings->get(&FilesSettings::supported_extensions);

    UniqueArray<AnyString> r;
    for (auto& path : paths) {
        if (fs::is_directory(path)) {
            usize old_size = r.size();
            for (auto& entry : fs::recursive_directory_iterator(path)) {
                auto child = AnyString(entry.path().u8string());
                usize i;
                for (i = child.size(); i > 0; --i) {
                    if (child[i-1] == '.') break;
                }
                Str ext = i ? child.substr(i) : "";
                if (!extensions.count(StaticString(ext))) continue;
                r.emplace_back(move(child));
            }
            std::sort(
                r.begin() + old_size, r.end(), &uni::natural_lessthan
            );
        }
        else {
             // Don't check the file extension for explicitly specified
             // files.
            r.emplace_back(path);
        }
    }
    return r;
}

static
UniqueArray<AnyString> read_list (Str path) {
    UniqueArray<AnyString> lines {""};
    if (path == "-") {
         // TODO: Make string_from_file support filehandles
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
        for (char c : string_from_file(path)) {
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

BookSource::BookSource (
    const Settings* settings, BookType t, const AnyString& n
) :
    type(t),
    name(fs::absolute(n).u8string())
{
    switch (type) {
        case BookType::Misc: require(false); never();
        case BookType::Folder: {
            pages = expand_recursively(settings, {name});
            break;
        }
        case BookType::FileWithNeighbors: {
            pages = expand_folder(settings, containing_folder(name));
            break;
        }
        case BookType::List: {
             // This requires CWD to be set to the folder containing the list.
             // TODO: Make this not the case.
            auto lines = read_list(name);
            pages = expand_recursively(settings, lines);
            break;
        }
        default: never();
    }
}

BookSource::BookSource (
    const Settings* settings, BookType t, Slice<AnyString> args
) :
    type(t), name()
{
    require(type == BookType::Misc);
    pages = expand_recursively(settings, args);
}

const AnyString& BookSource::name_for_memory () {
    static constexpr AnyString empty;
    switch (type) {
        case BookType::Misc: return empty;
        case BookType::Folder: return name;
        case BookType::List: return name == "-" ? empty : name;
        case BookType::FileWithNeighbors: return empty;
        default: never();
    }
}

} // liv
