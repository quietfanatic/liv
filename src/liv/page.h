// Implements the view of one image.

#pragma once

#include <memory>
#include "../dirt/geo/rect.h"
#include "../dirt/geo/scalar.h"
#include "../dirt/geo/vec.h"
#include "../dirt/glow/file-texture.h"
#include "../dirt/iri/iri.h"
#include "../dirt/uni/common.h"
#include "common.h"
#include "settings.h"

namespace liv {

struct RenderParams {
     // Set some default values.  We really should be getting these default
     // values from the settings, but it's awkward with the way we're currently
     // loading memories.
    InterpolationMode interpolation_mode = InterpolationMode::SmartCubic;
    Fill window_background = Fill::Black;
    Fill transparency_background = Fill::White;
    RenderParams () = default;
    RenderParams (const RenderParams&) = default;
    RenderParams (const Settings*);
};

struct Page {
    IRI location;
    std::unique_ptr<glow::FileTexture> texture;
    IVec size;
    isize estimated_memory = 0;
    double last_viewed_at = 0;
    bool load_failed = false;

    explicit Page (const IRI&);
    ~Page ();

    void load ();
    void unload ();

    void draw (
        RenderParams params,
        float zoom,
        const geo::Rect& screen_rect,
        const geo::Rect& tex_rect = GNAN // defaults to whole page
    );
};

} // namespace liv
