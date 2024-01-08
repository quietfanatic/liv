#include "page-block.h"

#include "../dirt/geo/scalar.h"
#include "book-source.h"
#include "book.h"
#include "page.h"

namespace liv {

PageBlock::PageBlock (const BookSource* src) :
    pages(src->pages.size(), [src](usize i){
        return std::make_unique<Page>(src->pages[i]);
    })
{ }
PageBlock::~PageBlock () { }

void PageBlock::source_updated (const BookSource* src) {
    std::unordered_map<Str, std::unique_ptr<Page>> by_iri;
    pages.consume([&by_iri](auto&& page) {
        by_iri.emplace(page->location.spec(), move(page));
    });
    pages = decltype(pages)(src->pages.size(), [src, &by_iri](usize i){
        auto iter = by_iri.find(src->pages[i].spec());
        if (iter != by_iri.end()) {
            auto r = move(iter->second);
            by_iri.erase(iter);
            return r;
        }
        else return std::make_unique<Page>(src->pages[i]);
    });
     // We need to explicitly unload any images that are left over because we're
     // keeping track of the estimated memory usage.
    for (auto& [_, page] : by_iri) {
        unload_page(&*page);
        page = {};
    }
}

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

bool PageBlock::idle_processing (const Book* book, const Settings& settings) {
    auto viewing = IRange{
        book->state.page_offset,
        book->state.page_offset + settings.get(&LayoutSettings::spread_count)
    };

     // Unload a cached page if we're minimized
    if (book->view.is_minimized()) {
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
