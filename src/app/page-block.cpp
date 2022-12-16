#include "page-block.h"

#include "../base/geo/scalar.h"
#include "files.h"
#include "page.h"

namespace app {

PageBlock::PageBlock (FilesToOpen& to_open) {
    AA(to_open.files.size() <= int32(GINF));
    pages.reserve(to_open.files.size());
    for (auto& filename : to_open.files) {
        pages.emplace_back(std::make_unique<Page>(std::move(filename)));
    }
}
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
        DA(estimated_page_memory >= 0);
    }
}

bool PageBlock::idle_processing (const Settings* settings, IRange viewing_range) {
    int32 preload_ahead = settings->get(&MemorySettings::preload_ahead);
    int32 preload_behind = settings->get(&MemorySettings::preload_behind);
    int32 page_cache_mb = settings->get(&MemorySettings::page_cache_mb);

    auto preload_range = IRange(
        viewing_range.l - preload_behind,
        viewing_range.r + preload_ahead
    ) & IRange(1, count());

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
        }
    }
     // Didn't do anything
    return false;
}

} // namespace app
