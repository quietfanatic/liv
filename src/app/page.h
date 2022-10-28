// Implements the view of one image.

#pragma once

#include "../base/geo/rect.h"
#include "../base/geo/vec.h"
#include "../base/glow/file-texture.h"
#include "../base/uni/common.h"

namespace app {

struct Page {
    String filename;
    glow::FileTexture texture;
    geo::IVec size;
    explicit Page (String filename);
    ~Page ();

    void draw (
        const geo::Rect& screen_rect,
        const geo::Rect& tex_rect = geo::NAN // defaults to whole page
    );
};

} // namespace app
