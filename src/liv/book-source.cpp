#include "book-source.h"

#include <filesystem>
#include "../dirt/iri/path.h"
#include "../dirt/uni/text.h"
#include "../dirt/uni/io.h"
#include "settings.h"

namespace liv {

static
UniqueArray<IRI> expand_neighbors (
    const Settings* settings, const IRI& loc
) {
    auto& extensions = settings->get(&FilesSettings::supported_extensions);
    UniqueArray<IRI> r;

    IRI folder = loc.without_filename();
    for (auto& entry : fs::directory_iterator(iri::to_fs_path(folder))) {
        auto child = iri::from_fs_path(Str(entry.path().generic_u8string()));
        Str ext = iri::path_extension(child.path());
        if (!extensions.count(StaticString(ext))) continue;
        r.emplace_back(move(child));
    }
    std::sort(r.begin(), r.end(),
        [](const IRI& a, const IRI& b){
            return uni::natural_lessthan(a.path(), b.path());
        }
    );
    return r;
}

static
UniqueArray<IRI> expand_recursively (
    const Settings* settings, Slice<IRI> locs
) {
    auto& extensions = settings->get(&FilesSettings::supported_extensions);
    UniqueArray<IRI> r;

    for (auto& loc : locs) {
        auto fs_path = iri::to_fs_path(loc);
        if (fs::is_directory(fs_path)) {
            usize old_size = r.size();
            for (auto& entry : fs::recursive_directory_iterator(fs_path)) {
                auto child = iri::from_fs_path(Str(entry.path().generic_u8string()));
                Str ext = iri::path_extension(child.path());
                if (!extensions.count(StaticString(ext))) continue;
                r.emplace_back(move(child));
            }
            std::sort(r.begin() + old_size, r.end(),
                [](const IRI& a, const IRI& b){
                    return uni::natural_lessthan(a.path(), b.path());
                }
            );
        }
        else {
             // Don't check the file extension for explicitly specified
             // files.
            r.emplace_back(loc);
        }
    }
    return r;
}

static
UniqueArray<IRI> read_list (const IRI& loc) {
    UniqueArray<IRI> r;

    UniqueString line;
    if (loc == "liv:stdin") {
         // TODO: Make string_from_file support filehandles
        for (;;) {
            int c = getchar();
            if (c == EOF) break;
            else if (c == '\n') {
                if (line) {
                    r.emplace_back(iri::from_fs_path(line));
                    line = "";
                }
            }
            else line.push_back(c);
        }
    }
    else {
        for (char c : string_from_file(iri::to_fs_path(loc))) {
            if (c == '\n') {
                if (line) {
                    r.emplace_back(iri::from_fs_path(line, loc));
                    line = "";
                }
            }
            else line.push_back(c);
        }
    }
    return r;
}

BookSource::BookSource (
    const Settings* settings, BookType t, const IRI& loc
) :
    type(t), location(loc)
{
    require(loc);
    switch (type) {
        case BookType::Misc: require(false); never();
        case BookType::Folder: {
            require(location.path().back() == '/');
            pages = expand_recursively(settings, {location});
            break;
        }
        case BookType::FileWithNeighbors: {
            require(location.path().back() != '/');
            pages = expand_neighbors(settings, location);
            break;
        }
        case BookType::List: {
            require(location.path().back() != '/');
            auto entries = read_list(location);
            pages = expand_recursively(settings, entries);
            break;
        }
        default: never();
    }
}

BookSource::BookSource (
    const Settings* settings, BookType t, Slice<IRI> args
) :
    type(t), location()
{
    require(type == BookType::Misc);
    pages = expand_recursively(settings, args);
}

const IRI& BookSource::location_for_memory () {
    static constexpr IRI empty;
    switch (type) {
        case BookType::Misc: return empty;
        case BookType::Folder: return location;
        case BookType::List: return location == "liv:stdin" ? empty : location;
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

    auto settings = &builtin_default_settings;

    auto here = IRI("res/liv/", iri::program_location());

    BookSource misc_src (settings, BookType::Misc, Slice<IRI>{
        iri::from_fs_path("test/image.png", here),
        iri::from_fs_path("test/image2.png", here),
        iri::from_fs_path("test/non-image.txt", here),
        iri::from_fs_path("test/", here)
    });
    is(misc_src.pages.size(), 5u, "BookType::Misc");
    is(misc_src.pages[0].relative_to(here), "test/image.png", "BookType::Misc 0");
    is(misc_src.pages[1].relative_to(here), "test/image2.png", "BookType::Misc 1");
    is(misc_src.pages[2].relative_to(here), "test/non-image.txt", "BookType::Misc 2");
    todo(2, "order is nonintuitive");
    is(misc_src.pages[3].relative_to(here), "test/image.png", "BookType::Misc 3");
    is(misc_src.pages[4].relative_to(here), "test/image2.png", "BookType::Misc 4");
    is(misc_src.location_for_memory(), "", "BookType::Misc shouldn't be remembered");

    BookSource folder_src (settings, BookType::Folder, iri::from_fs_path("test/", here));
    is(folder_src.pages.size(), 2u, "BookType::Folder");
    todo(2, "order is nonintuitive");
    is(folder_src.pages[0].relative_to(here), "test/image.png", "BookType::Folder 0");
    is(folder_src.pages[1].relative_to(here), "test/image2.png", "BookType::Folder 1");
    is(folder_src.location_for_memory().relative_to(here), "test/", "BookType::Folder name for memory");

    BookSource file_src (settings, BookType::FileWithNeighbors, iri::from_fs_path("test/image2.png", here));
    is(file_src.pages.size(), 2u, "BookType::FileWithNeighbors");
    todo(2, "order is nonintuitive");
    is(file_src.pages[0].relative_to(here), "test/image.png", "BookType::FilewithNeighbors 0");
    is(file_src.pages[1].relative_to(here), "test/image2.png", "BookType::FilewithNeighbors 1");
    is(file_src.location_for_memory(), "", "BookType::FileWithNeighbors shouldn't be remembered");

    BookSource list_src (settings, BookType::List, iri::from_fs_path("test/list.lst", here));
    is(list_src.pages.size(), 2u, "BookType::List");
     // Intentionally backwards
    is(list_src.pages[0].relative_to(here), "test/image2.png", "BookType::List 0");
    is(list_src.pages[1].relative_to(here), "test/image.png", "BookType::List 1");
    is(list_src.location_for_memory().relative_to(here), "test/list.lst", "BookType::List name for memory");

    done_testing();
});
#endif

