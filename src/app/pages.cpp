#include "pages.h"

#include "../base/geo/scalar.h"
#include "files.h"
#include "page.h"

namespace app {

Pages::Pages (FilesToOpen& to_open) {
    AA(to_open.files.size() <= isize(MAX));
    pages.reserve(to_open.files.size());
    for (auto& filename : to_open.files) {
        pages.emplace_back(std::make_unique<Page>(std::move(filename)));
    }
}
Pages::~Pages () { }

isize Pages::clamp_page_offset (PageRange range) const {
    if (count() > 0) return clamp(
        range.offset, 1 - (range.spread_pages-1), count()
    );
    else return 1;
}

isize Pages::first_visible_page (PageRange range) const {
    return max(range.offset, 1);
}
isize Pages::last_visible_page (PageRange range) const {
    return min(range.offset + range.spread_pages-1, count());
}

Page* Pages::get (isize no) const {
    if (no < 1 || no > count()) return null;
    else return &*pages[no-1];
}

void Pages::load_page (Page* page) {
    if (page && !page->texture) {
        page->load();
        estimated_page_memory += page->estimated_memory;
    }
}

void Pages::unload_page (Page* page) {
    if (page && page->texture) {
        page->unload();
        estimated_page_memory -= page->estimated_memory;
        DA(estimated_page_memory >= 0);
    }
}

bool Pages::idle_processing (const Settings* settings, PageRange range) {
    int32 preload_ahead = settings->get(&MemorySettings::preload_ahead);
    int32 preload_behind = settings->get(&MemorySettings::preload_behind);
    int32 page_cache_mb = settings->get(&MemorySettings::page_cache_mb);

    isize preload_first = max(range.offset - preload_behind, 1);
    isize preload_last = min(
        range.offset + range.spread_pages - 1 + preload_ahead,
        count()
    );
     // Preload pages forwards
    for (isize no = range.offset; no >= preload_first; no--) {
        if (Page* page = get(no)) {
            if (!page->texture && !page->load_failed) {
                load_page(page);
                return true;
            }
        }
    }
     // Preload pages backwards
    for (isize no = range.offset + range.spread_pages - 1; no <= preload_last; no++) {
        if (Page* page = get(no)) {
            if (!page->texture && !page->load_failed) {
                load_page(page);
                return true;
            }
        }
    }
     // Unload pages if we're above the memory limit
    isize limit = page_cache_mb * (1024*1024);
    if (estimated_page_memory > limit) {
        double oldest_viewed_at = INF;
        Page* oldest_page = null;
        for (isize no = 1; no <= count(); no++) {
             // Don't unload images in the preload region
            if (no >= preload_first && no <= preload_last) {
                continue;
            }
            Page* page = get(no);
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
