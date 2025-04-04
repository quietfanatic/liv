#include "page-block.h"

#include <fcntl.h>
#include "../dirt/geo/scalar.h"
#include "../dirt/uni/io.h"
#include "../dirt/uni/text.h"
#include "book-source.h"
#include "book.h"
#include "list.h"
#include "page.h"

namespace liv {

NOINLINE static
UniqueArray<IRI> expand_neighbors (
    const Settings& settings, const IRI& loc
) {
    plog("expanding neighbors");
    auto& extensions = settings.get(&FilesSettings::page_extensions);
    IRI folder = loc.chop_filename();
    Str self = iri::path_filename(loc.path());

    UniqueArray<IRI> r;

    for (Str child : Dir(iri::to_fs_path(folder))) {
        expect(child[0]);
        if (child[0] == '.') continue;
         // Don't check extension if we explicitly requested the file.
        if (child != self) {
            auto ext = ascii_to_lower(iri::path_extension(child));
            for (auto& e : extensions) {
                if (e == ext) goto pick;
            }
            continue;
        }
        pick:
        IRI neighbor = iri::from_fs_path(child, folder);
        expect(neighbor);
        r.emplace_back(move(neighbor));
    };

    auto sort = settings.get(&FilesSettings::sort);
    sort_iris(r.begin(), r.end(), sort);
    return r;
}

void expand_recursively_recurse (
    UniqueArray<IRI>& r,
    Slice<AnyString> extensions,
    Dir& dir,
    const IRI& folder
) {
    for (Str child : dir) {
        expect(child);
        if (child[0] == '.') continue;
         // TODO: reduce string copies
        if (Dir subdir = Dir::try_open_at(dir.fd, child)) {
            IRI subfolder = iri::from_fs_path(cat(subdir.path, '/'), folder);
            expect(subfolder);
            expand_recursively_recurse(
                r, extensions, subdir, subfolder
            );
        }
        else {
             // Ignore failure to open, delay it for when we load the page.
            auto ext = ascii_to_lower(iri::path_extension(child));
            for (auto& e : extensions) {
                if (e == ext) goto pick;
            }
            continue;
            pick:
            IRI neighbor = iri::from_fs_path(child, folder);
            expect(neighbor);
            r.emplace_back(move(neighbor));
        }
    }
}


NOINLINE static
UniqueArray<IRI> expand_recursively (
    const Settings& settings, Slice<IRI> locs, BookType type
) {
    plog("expanding recursively");

    auto& extensions = settings.get(&FilesSettings::page_extensions);
    auto sort = settings.get(&FilesSettings::sort);
    bool sort_everything;
    switch (type) {
        case BookType::Misc: {
            sort_everything = !(sort.flags & SortFlags::NotArgs);
            break;
        }
        case BookType::Folder: {
            sort_everything = true;
            break;
        }
        case BookType::List: {
            sort_everything = !(sort.flags & SortFlags::NotLists);
            break;
        }
        default: never();
    }

    UniqueArray<IRI> r;
    for (auto& loc : locs) {
        auto path = iri::to_fs_path(loc);
        if (Dir dir = Dir::try_open_at(AT_FDCWD, path)) {
            IRI folder = IRI(cat(dir.path, '/'), "file:");
            usize old_size = r.size();
            expand_recursively_recurse(
                r, extensions, dir, folder
            );
            if (!sort_everything) {
                sort_iris(r.begin() + old_size, r.end(), sort);
            }
        }
        else {
             // Don't check the file extension or hiddenness for explicitly
             // specified files.
            r.emplace_back(loc);
        }
    }
    if (sort_everything) {
        sort_iris(r.begin(), r.end(), sort);
    }
    return r;
}

PageBlock::PageBlock (const BookSource& src, const Settings& settings) {
    UniqueArray<IRI> locs;
    switch (src.type) {
        case BookType::Misc: {
            locs = expand_recursively(settings, src.locations, src.type);
            break;
        }
        case BookType::Folder: {
            locs = expand_recursively(settings, src.locations, src.type);
            break;
        }
        case BookType::List: {
            locs = read_list(src.locations[0]);
            locs = expand_recursively(settings, locs, src.type);
            break;
        }
        case BookType::FileWithNeighbors: {
            locs = expand_neighbors(settings, src.locations[0]);
            break;
        }
        default: never();
    }
    pages = decltype(pages)(locs.size(), [&](usize i){
        return std::make_unique<Page>(locs[i]);
    });
}
PageBlock::~PageBlock () { }

void PageBlock::resort (SortMethod method) {
     // Make array of just IRIs for sorting
    auto locs = UniqueArray<IRI>(pages.size(), [this](usize i){
        return pages[i]->location;
    });
     // Do the sort
    sort_iris(locs.begin(), locs.end(), method);
     // Normally pages are indexed by offset, but we need to temporarily index
     // them by location.
    std::unordered_map<Str, std::unique_ptr<Page>> by_loc;
    pages.consume([&by_loc](auto&& page) {
        by_loc.emplace(page->location.spec(), move(page));
    });
     // Reorder pages, reusing existing objects
    pages = decltype(pages)(locs.size(), [&](usize i){
        auto iter = by_loc.find(locs[i].spec());
        if (iter != by_loc.end()) {
            auto r = move(iter->second);
            by_loc.erase(iter);
            return r;
        }
        else return std::make_unique<Page>(locs[i]);
    });
     // We need to explicitly unload any images that are left over because we're
     // keeping track of the estimated memory usage.
    for (auto& [_, page] : by_loc) {
        unload_page(&*page);
        page = {};
    }
}

Page* PageBlock::get (int32 i) const {
    if (i < 0 || i >= count()) return null;
    else return &*pages[i];
}

int32 PageBlock::find (const IRI& loc) const {
    if (!loc) return -1;
    for (int32 i = 0; i < int32(pages.size()); i++) {
        if (pages[i]->location == loc) return i;
    }
    return -1;
}

void PageBlock::load_page (Page* page) {
    if (page && !page->texture) {
        page->load();
        estimated_page_memory += page->estimated_memory;
    }
}

void PageBlock::unload_page (Page* page) {
    if (page && page->texture) {
        page->unload();
        estimated_page_memory -= page->estimated_memory;
        expect(estimated_page_memory >= 0);
    }
}

bool PageBlock::idle_processing (const Book* book, const Settings& settings) {
    auto viewing = IRange{
        book->state.page_offset,
        book->state.page_offset + settings.get(&LayoutSettings::spread_count)
    };

     // Unload a cached page if we're minimized
    if (book->view.window.is_minimized()) {
        switch (settings.get(&MemorySettings::trim_when_minimized)) {
            case TrimMode::None: break;
            case TrimMode::PageCache: {
                for (int32 i = 0; i < viewing.l; ++i) {
                    Page* page = get(i);
                    if (page->texture) {
                        unload_page(page);
                        return true;
                    }
                }
                for (int32 i = viewing.r; i < count(); ++i) {
                    Page* page = get(i);
                    if (page->texture) {
                        unload_page(page);
                        return true;
                    }
                }
                return false;
            }
        }
    }
     // Otherwise continue as normal...

    int32 preload_ahead = settings.get(&MemorySettings::preload_ahead);
    int32 preload_behind = settings.get(&MemorySettings::preload_behind);
    int32 page_cache_mb = settings.get(&MemorySettings::page_cache_mb);

    auto preload_range = IRange(
        viewing.l - preload_behind,
        viewing.r + preload_ahead
    ) & IRange(0, count());

     // Preload pages forwards
    for (int32 i = viewing.r; i < preload_range.r; i++) {
        if (Page* page = get(i)) {
            if (!page->texture && !page->load_failed) {
                load_page(page);
                return true;
            }
        }
    }
     // Preload pages backwards
    for (int32 i = viewing.l - 1; i > preload_range.l - 1; i--) {
        if (Page* page = get(i)) {
            if (!page->texture && !page->load_failed) {
                load_page(page);
                return true;
            }
        }
    }
     // Unload a page if we're above the memory limit
    int64 limit = page_cache_mb * int64(1024*1024);
    if (estimated_page_memory > limit) {
        double oldest_viewed_at = GINF;
        Page* oldest_page = null;
        for (int32 i = 0; i < count(); i++) {
             // Don't unload pages in the preload region, or we'll keep loading
             // and unloading them forever.
            if (contains(preload_range, i)) continue;
            Page* page = get(i);
            if (!page->texture) continue;
            if (page->last_viewed_at < oldest_viewed_at) {
                oldest_viewed_at = page->last_viewed_at;
                oldest_page = page;
            }
        }
        if (oldest_page) {
            unload_page(oldest_page);
            return true;
        }
    }
     // Didn't do anything
    return false;
}

} // namespace liv

#ifndef TAP_DISABLE_TESTS
#include <filesystem>
#include <SDL2/SDL.h>
#include "../dirt/tap/tap.h"

static tap::TestSet tests ("liv/page-block", []{
    using namespace tap;
    using namespace liv;

    auto settings = &builtin_default_settings;

    auto here = IRI("res/liv/", iri::program_location());

    BookSource misc_src {BookType::Misc, Slice<IRI>{
        iri::from_fs_path("test/image.png", here),
        iri::from_fs_path("test/image2.png", here),
        iri::from_fs_path("test/non-image.txt", here),
        iri::from_fs_path("test/", here)
    }};
    PageBlock misc_block {misc_src, *settings};
    is(misc_block.pages.size(), 5u, "BookType::Misc");
    is(misc_block.pages[0]->location.relative_to(here), "test/image.png", "BookType::Misc 0");
    is(misc_block.pages[1]->location.relative_to(here), "test/image2.png", "BookType::Misc 1");
    is(misc_block.pages[2]->location.relative_to(here), "test/non-image.txt", "BookType::Misc 2");
    is(misc_block.pages[3]->location.relative_to(here), "test/image.png", "BookType::Misc 3");
    is(misc_block.pages[4]->location.relative_to(here), "test/image2.png", "BookType::Misc 4");
    ok(misc_src.location_for_mark().empty(), "BookType::Misc shouldn't be remembered");

    BookSource folder_src {BookType::Folder, {iri::from_fs_path("test/", here)}};
    PageBlock folder_block {folder_src, *settings};
    is(folder_block.pages.size(), 2u, "BookType::Folder");
    is(folder_block.pages[0]->location.relative_to(here), "test/image.png", "BookType::Folder 0");
    is(folder_block.pages[1]->location.relative_to(here), "test/image2.png", "BookType::Folder 1");
    is(folder_src.location_for_mark().relative_to(here), "test/", "BookType::Folder name for mark");

    BookSource file_src {BookType::FileWithNeighbors, {iri::from_fs_path("test/image2.png", here)}};
    PageBlock file_block {file_src, *settings};
    is(file_block.pages.size(), 2u, "BookType::FileWithNeighbors");
    is(file_block.pages[0]->location.relative_to(here), "test/image.png", "BookType::FilewithNeighbors 0");
    is(file_block.pages[1]->location.relative_to(here), "test/image2.png", "BookType::FilewithNeighbors 1");
    ok(file_src.location_for_mark().empty(), "BookType::FileWithNeighbors shouldn't be remembered");

    BookSource list_src {BookType::List, {iri::from_fs_path("test/list.lst", here)}};
    PageBlock list_block {list_src, *settings};
    is(list_block.pages.size(), 2u, "BookType::List");
     // Intentionally backwards
    is(list_block.pages[0]->location.relative_to(here), "test/image2.png", "BookType::List 0");
    is(list_block.pages[1]->location.relative_to(here), "test/image.png", "BookType::List 1");
    is(list_src.location_for_mark().relative_to(here), "test/list.lst", "BookType::List name for mark");

    done_testing();
});
#endif
