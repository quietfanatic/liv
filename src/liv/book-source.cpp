#include "book-source.h"

#include <filesystem>
#include "../dirt/iri/path.h"
#include "list.h"
#include "settings.h"
#include "sort.h"

namespace liv {

static
UniqueArray<IRI> expand_neighbors (
    const Settings* settings, const IRI& loc, SortMethod sort
) {
    auto& extensions = settings->get(&FilesSettings::page_extensions);
    UniqueArray<IRI> r;

    IRI folder = loc.without_filename();
    for (auto& entry : fs::directory_iterator(iri::to_fs_path(folder))) {
        auto child = iri::from_fs_path(Str(entry.path().generic_u8string()));
        Str ext = iri::path_extension(child.path());
        if (!extensions.count(StaticString(ext))) continue;
        r.emplace_back(move(child));
    }
    do_sort(r.begin(), r.end(), sort);
    return r;
}

static
UniqueArray<IRI> expand_recursively (
    const Settings* settings, Slice<IRI> locs, SortMethod sort
) {
    auto& extensions = settings->get(&FilesSettings::page_extensions);
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
            if (sort) {
                do_sort(r.begin() + old_size, r.end(), sort);
            }
        }
        else {
             // Don't check the file extension for explicitly specified
             // files.
            r.emplace_back(loc);
        }
    }
    return r;
}

BookSource::BookSource (
    const Settings* settings,
    BookType t, const IRI& loc,
    SortMethod sort
) :
    type(t), location(loc)
{
    if (!sort) {
        sort = settings->get(&FilesSettings::sort);
    }
    require(loc);
    switch (type) {
        case BookType::Misc: require(false); never();
        case BookType::Folder: {
            require(location.path().back() == '/');
            pages = expand_recursively(settings, {location}, sort);
            break;
        }
        case BookType::FileWithNeighbors: {
            require(location.path().back() != '/');
            pages = expand_neighbors(settings, location, sort);
            break;
        }
        case BookType::List: {
            require(location.path().back() != '/');
            auto entries = read_list(location);
            if (!!(sort.flags & SortFlags::NotLists)) {
                pages = expand_recursively(settings, entries, sort);
            }
            else {
                pages = expand_recursively(settings, entries, SortMethod{});
                do_sort(pages.begin(), pages.end(), sort);
            }
            break;
        }
        default: never();
    }
}

BookSource::BookSource (
    const Settings* settings, BookType t, Slice<IRI> args, SortMethod sort
) :
    type(t), location()
{
    if (!sort) {
        sort = settings->get(&FilesSettings::sort);
    }
    require(type == BookType::Misc);
    if (!!(sort.flags & SortFlags::NotArgs)) {
        pages = expand_recursively(settings, args, sort);
    }
    else {
        pages = expand_recursively(settings, args, SortMethod{});
        do_sort(pages.begin(), pages.end(), sort);
    }
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
    is(misc_src.pages[3].relative_to(here), "test/image.png", "BookType::Misc 3");
    is(misc_src.pages[4].relative_to(here), "test/image2.png", "BookType::Misc 4");
    is(misc_src.location_for_memory(), "", "BookType::Misc shouldn't be remembered");

    BookSource folder_src (settings, BookType::Folder, iri::from_fs_path("test/", here));
    is(folder_src.pages.size(), 2u, "BookType::Folder");
    is(folder_src.pages[0].relative_to(here), "test/image.png", "BookType::Folder 0");
    is(folder_src.pages[1].relative_to(here), "test/image2.png", "BookType::Folder 1");
    is(folder_src.location_for_memory().relative_to(here), "test/", "BookType::Folder name for memory");

    BookSource file_src (settings, BookType::FileWithNeighbors, iri::from_fs_path("test/image2.png", here));
    is(file_src.pages.size(), 2u, "BookType::FileWithNeighbors");
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

