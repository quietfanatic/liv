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
    UniqueArray<AnyString> lines;
    UniqueString line;
    if (path == "-") {
         // TODO: Make string_from_file support filehandles
        for (;;) {
            int c = getchar();
            if (c == EOF) break;
            else if (c == '\n') {
                if (line) lines.emplace_back(move(line));
            }
            else line.push_back(c);
        }
    }
    else {
        for (char c : string_from_file(path)) {
            if (c == '\n') {
                if (line) lines.emplace_back(move(line));
            }
            else line.push_back(c);
        }
    }
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

#ifndef TAP_DISABLE_TESTS
#include <filesystem>
#include <SDL2/SDL.h>
#include "../dirt/tap/tap.h"

static tap::TestSet tests ("liv/book-source", []{
    using namespace tap;
    using namespace liv;

    char* base = glow::require_sdl(SDL_GetBasePath());
    auto exe_folder = UniqueString(base);
    SDL_free(base);

    auto settings = &builtin_default_settings;

    fs::current_path(cat(exe_folder, "/res/liv"));

    BookSource misc_src (settings, BookType::Misc, Slice<AnyString>{
        "test/image.png",
        "test/image2.png",
        "test/non-image.txt",
        "test"
    });
    is(misc_src.pages.size(), 5u, "BookType::Misc");
    is(misc_src.pages[0], "test/image.png", "BookType::Misc 0");
    is(misc_src.pages[1], "test/image2.png", "BookType::Misc 1");
    is(misc_src.pages[2], "test/non-image.txt", "BookType::Misc 2");
    todo(2, "order is nonintuitive");
    is(misc_src.pages[3], "test/image.png", "BookType::Misc 3");
    is(misc_src.pages[4], "test/image2.png", "BookType::Misc 4");
    is(misc_src.name_for_memory(), "", "BookType::Misc shouldn't be remembered");

    todo("absolute/relative failures", [&]{
        BookSource folder_src (settings, BookType::Folder, "test");
        is(folder_src.pages.size(), 2u, "BookType::Folder");
        is(folder_src.pages[0], "test/image.png", "BookType::Folder 0");
        is(folder_src.pages[1], "test/image2.png", "BookType::Folder 1");
        is(folder_src.name_for_memory(),
            cat(exe_folder, "/res/liv/test"),
            "BookType::Folder name for memory"
        );

        BookSource file_src (settings, BookType::FileWithNeighbors, "test/image2.png");
        is(file_src.pages.size(), 2u, "BookType::FileWithNeighbors");
        is(file_src.pages[0], "test/image.png", "BookType::FilewithNeighbors 0");
        is(file_src.pages[1], "test/image2.png", "BookType::FilewithNeighbors 1");
        is(file_src.name_for_memory(), "", "BookType::FileWithNeighbors shouldn't be remembered");

         // Do this one last because it chdirs (TODO: don't)
        BookSource list_src (settings, BookType::List, "test/list.lst");
        is(list_src.pages.size(), 2u, "BookType::List");
         // Intentionally backwards
        is(list_src.pages[0], "test/image2.png", "BookType::List 0");
        is(list_src.pages[1], "test/image.png", "BookType::List 1");
        is(list_src.name_for_memory(),
            cat(exe_folder, "/rest/liv/test/list.lst"),
            "BookType::List name for memory"
        );
    });

    done_testing();
});
#endif

