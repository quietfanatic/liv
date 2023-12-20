// Implements the view of one image.

#pragma once

#include <memory>
#include "../dirt/geo/rect.h"
#include "../dirt/geo/scalar.h"
#include "../dirt/geo/vec.h"
#include "../dirt/glow/file-texture.h"
#include "../dirt/uni/common.h"
#include "common.h"
#include "settings.h"

namespace app {

struct PageParams {
    InterpolationMode interpolation_mode;
    PageParams () = default;
    PageParams (const PageParams&) = default;
    PageParams (const Settings*);
};

struct Page {
    AnyString filename;
    std::unique_ptr<glow::FileTexture> texture;
    IVec size;
    isize estimated_memory = 0;
    double last_viewed_at = 0;
    bool load_failed = false;

    explicit Page (AnyString filename);
    ~Page ();

    void load ();
    void unload ();

    void draw (
        PageParams params,
        float zoom,
        const geo::Rect& screen_rect,
        const geo::Rect& tex_rect = GNAN // defaults to whole page
    );
};

} // namespace app
