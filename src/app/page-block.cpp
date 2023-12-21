#include "page-block.h"

#include "../dirt/geo/scalar.h"
#include "book.h"
#include "files.h"
#include "page.h"

namespace app {

PageBlock::PageBlock (
    const AnyString& book_filename,
    Slice<AnyString> page_filenames
) :
    book_filename(book_filename),
    pages(page_filenames.size(), [&](usize i){
        return std::make_unique<Page>(page_filenames[i]);
    })
{ }
PageBlock::~PageBlock () { }

Page* PageBlock::get (int32 i) const {
    if (i < 0 || i >= count()) return null;
    else return &*pages[i];
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

bool PageBlock::idle_processing (const Book* book, const Settings* settings) {
    auto viewing_range = book->viewing_pages;

     // Unload a cached page if we're minimized
    if (book->is_minimized()) {
        switch (settings->get(&MemorySettings::trim_when_minimized)) {
            case TRIM_NONE: break;
            case TRIM_PAGE_CACHE: {
                for (int32 i = 0; i < viewing_range.l; ++i) {
                    Page* page = get(i);
                    if (page->texture) {
                        unload_page(page);
                        return true;
                    }
                }
                for (int32 i = viewing_range.r; i < count(); ++i) {
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

    int32 preload_ahead = settings->get(&MemorySettings::preload_ahead);
    int32 preload_behind = settings->get(&MemorySettings::preload_behind);
    int32 page_cache_mb = settings->get(&MemorySettings::page_cache_mb);

    auto preload_range = IRange(
        viewing_range.l - preload_behind,
        viewing_range.r + preload_ahead
    ) & IRange(0, count());

     // Preload pages forwards
    for (int32 i = viewing_range.r; i < preload_range.r; i++) {
        if (Page* page = get(i)) {
            if (!page->texture && !page->load_failed) {
                load_page(page);
                return true;
            }
        }
    }
     // Preload pages backwards
    for (int32 i = viewing_range.l - 1; i > preload_range.l - 1; i--) {
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

} // namespace app
