// Implements the view of one image.

#pragma once

#include <memory>
#include "../base/geo/rect.h"
#include "../base/geo/vec.h"
#include "../base/glow/file-texture.h"
#include "../base/uni/common.h"
#include "settings.h"

namespace app {

struct Page {
    String filename;
    std::unique_ptr<glow::FileTexture> texture;
    geo::IVec size;
    isize estimated_memory = 0;
    double last_viewed_at = 0;
    bool load_failed = false;

    explicit Page (String filename);
    ~Page ();

    void load ();
    void unload ();

    void draw (
        InterpolationMode interpolation_mode,
        float zoom,
        const geo::Rect& screen_rect,
        const geo::Rect& tex_rect = geo::NAN // defaults to whole page
    );
};

} // namespace app
