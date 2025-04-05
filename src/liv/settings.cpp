#include "settings.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/uni/text.h"

namespace liv {

static AnyString extensions [] = {
    "bmp", "gif", "jfif", "jpe", "jpeg", "jpg",
    "png", "tif", "tiff", "xbm", "xpm", "webp",
};

 // TODO: reconst this somehow
Settings builtin_default_settings = {
    .parent = null,
    .window = {
        .size = {IVec{720, 720}},
        .fullscreen = {false},
        .title = {FormatList(
            FormatToken("["), FormatToken(FormatCommand::VisibleRange),
            FormatToken("/"), FormatToken(FormatCommand::PageCount),
            FormatToken("] "), FormatToken(FormatCommand::PageRelCwd),
            FormatToken(FormatCommand::IfZoomed, FormatList(
                FormatToken(" ("), FormatToken(FormatCommand::ZoomPercent),
                FormatToken("%)")
            ))
        )},
        .hidden = {false},
        .automated_input = {false},
        .last_prompt_command = {""},
    },
    .layout = {
        .spread_count = {1},
        .spread_direction = {Direction::Right},
        .auto_zoom_mode = {AutoZoomMode::Fit},
        .max_zoom = {32},
        .min_zoomed_size = {16},
        .reset_on_seek = {ResetOnSeek::Zoom},
        .small_align = {Vec{0.5, 0.5}},
        .large_align = {Vec{0.5, 0.5}},
        .scroll_margin = {0},
        .orientation = {Direction::Up},
    },
    .render = {
        .upscaler = {Upscaler::CubicRingless},
        .downscaler = {Downscaler::Squares9},
        .window_background = {Fill::Black},
        .transparency_background = {Fill::White},
        .color_range = {ColorRange{Vec3{0, 0, 0}, Vec3{1, 1, 1}}},
    },
    .control = {
        .scroll_speed = {Vec{20, 20}},
        .drag_speed = {Vec{1, 1}},
    },
    .files = {
        .sort = {SortMethod{
            SortCriterion::Natural, SortFlags::NotArgs | SortFlags::NotLists
        }},
        .page_extensions = {StaticArray<AnyString>(
            extensions, sizeof(extensions)/sizeof(extensions[0])
        )},
    },
    .memory = {
        .preload_ahead = {1},
        .preload_behind = {1},
        .page_cache_mb = {200},
        .trim_when_minimized = {TrimMode::PageCache},
    },
    .mappings = { },
};

void Settings::canonicalize () {
    if (files.page_extensions) {
        for (auto& e : *files.page_extensions) {
            for (auto& c : e) {
                if (c >= 'A' && c <= 'Z') {
                    goto canonicalize_extensions;
                }
            }
        }
        goto dont_canonicalize_extensions;
        canonicalize_extensions:
        for (auto& e : files.page_extensions->mut_slice()) {
            for (auto& c : e) {
                if (c >= 'A' && c <= 'Z') {
                    e = ascii_to_lower(e);
                    break;
                }
            }
        }
        dont_canonicalize_extensions:;
    }
}

void Settings::merge (Settings&& o) {
     // Is there a better solution than this?
#define LIV_MERGE(p) if (o.p) p = move(o.p);
    LIV_MERGE(window.size)
    LIV_MERGE(window.fullscreen)
    LIV_MERGE(window.title)
    LIV_MERGE(window.hidden)
    LIV_MERGE(window.automated_input)
    LIV_MERGE(window.last_prompt_command)
    LIV_MERGE(layout.spread_count)
    LIV_MERGE(layout.spread_direction)
    LIV_MERGE(layout.auto_zoom_mode)
    LIV_MERGE(layout.max_zoom)
    LIV_MERGE(layout.min_zoomed_size)
    LIV_MERGE(layout.reset_on_seek)
    LIV_MERGE(layout.scroll_margin)
    LIV_MERGE(layout.small_align)
    LIV_MERGE(layout.large_align)
    LIV_MERGE(layout.orientation)
    LIV_MERGE(render.upscaler)
    LIV_MERGE(render.downscaler)
    LIV_MERGE(render.window_background)
    LIV_MERGE(render.transparency_background)
    LIV_MERGE(render.color_range)
    LIV_MERGE(control.drag_speed)
    LIV_MERGE(control.scroll_speed)
    LIV_MERGE(files.sort)
    LIV_MERGE(files.page_extensions)
    LIV_MERGE(memory.preload_ahead)
    LIV_MERGE(memory.preload_behind)
    LIV_MERGE(memory.page_cache_mb)
    LIV_MERGE(memory.trim_when_minimized)
#undef LIV_MERGE
    mappings.reserve(mappings.size() + o.mappings.size());
    o.mappings.consume([this](Mapping&& m){
        mappings.emplace_back_expect_capacity(move(m));
    });
    if (o.parent != &builtin_default_settings) parent = o.parent;
}

control::Statement* Settings::map_input (control::Input input) {
    for (auto& [binding, action] : mappings) {
        if (input_matches_binding(input, binding)) {
            return &action;
        }
    }
    if (parent) return parent->map_input(input);
    else return null;
}

} using namespace liv;

AYU_DESCRIBE(liv::Fill,
    values(
        value("black", Fill::Black),
        value("white", Fill::White),
        value("transparent", Fill::Transparent)
    ),
    delegate(base<glow::RGBA8>())
)

AYU_DESCRIBE(liv::AutoZoomMode,
    values(
        value("fit", AutoZoomMode::Fit),
        value("fit_width", AutoZoomMode::FitWidth),
        value("fit_height", AutoZoomMode::FitHeight),
        value("original", AutoZoomMode::Original)
    )
)

AYU_DESCRIBE(liv::ResetOnSeek,
    values(
        value("none", ResetOnSeek::None),
        value("offset", ResetOnSeek::Offset),
        value("zoom", ResetOnSeek::Zoom)
    )
)

AYU_DESCRIBE(liv::Upscaler,
    values(
        value("nearest", Upscaler::Nearest),
        value("linear", Upscaler::Linear),
        value("cubic", Upscaler::Cubic),
        value("cubic_ringless", Upscaler::CubicRingless),
        value("smoothed", Upscaler::Smoothed)
    )
)

AYU_DESCRIBE(liv::Downscaler,
    values(
        value("nearest", Downscaler::Nearest),
        value("linear", Downscaler::Linear),
        value("squares9", Downscaler::Squares9)
    )
)

AYU_DESCRIBE(liv::Direction,
    values(
        value("right", Direction::Right),
        value("left", Direction::Left),
        value("down", Direction::Down),
        value("up", Direction::Up)
    )
)

AYU_DESCRIBE(liv::TrimMode,
    values(
        value("none", TrimMode::None),
        value("page_cache", TrimMode::PageCache)
    )
)

AYU_DESCRIBE(liv::Mapping,
    elems(
        elem(&Mapping::input),
        elem(&Mapping::action)
    )
)

AYU_DESCRIBE(liv::WindowSettings,
    attrs(
        attr("size", &WindowSettings::size, collapse_optional),
        attr("fullscreen", &WindowSettings::fullscreen, collapse_optional),
        attr("title", &WindowSettings::title, collapse_optional),
        attr("last_prompt_command", &WindowSettings::last_prompt_command, collapse_optional)
    )
)

AYU_DESCRIBE(liv::LayoutSettings,
    attrs(
        attr("spread_count", &LayoutSettings::spread_count, collapse_optional),
        attr("spread_direction", &LayoutSettings::spread_direction, collapse_optional),
        attr("auto_zoom_mode", &LayoutSettings::auto_zoom_mode, collapse_optional),
        attr("reset_on_seek", &LayoutSettings::reset_on_seek, collapse_optional),
        attr("max_zoom", &LayoutSettings::max_zoom, collapse_optional),
        attr("min_zoomed_size", &LayoutSettings::min_zoomed_size, collapse_optional),
        attr("small_align", &LayoutSettings::small_align, collapse_optional),
        attr("large_align", &LayoutSettings::large_align, collapse_optional),
        attr("scroll_margin", &LayoutSettings::scroll_margin, collapse_optional),
        attr("orientation", &LayoutSettings::orientation, collapse_optional)
    )
)
AYU_DESCRIBE(liv::RenderSettings,
    attrs(
        attr("upscaler", &RenderSettings::upscaler, collapse_optional),
        attr("downscaler", &RenderSettings::downscaler, collapse_optional),
        attr("window_background", &RenderSettings::window_background, collapse_optional),
        attr("transparency_background", &RenderSettings::transparency_background, collapse_optional),
        attr("color_range", &RenderSettings::color_range, collapse_optional)
    )
)

AYU_DESCRIBE(liv::ControlSettings,
    attrs(
        attr("drag_speed", &ControlSettings::drag_speed, collapse_optional),
        attr("scroll_speed", &ControlSettings::scroll_speed, collapse_optional)
    )
)

AYU_DESCRIBE(liv::FilesSettings,
    attrs(
        attr("sort", &FilesSettings::sort, collapse_optional),
        attr("page_extensions", &FilesSettings::page_extensions, collapse_optional)
    )
)

AYU_DESCRIBE(liv::MemorySettings,
    attrs(
        attr("preload_ahead", &MemorySettings::preload_ahead, collapse_optional),
        attr("preload_behind", &MemorySettings::preload_behind, collapse_optional),
        attr("page_cache_mb", &MemorySettings::page_cache_mb, collapse_optional),
        attr("trim_when_minimized", &MemorySettings::trim_when_minimized, collapse_optional)
    )
)

AYU_DESCRIBE(liv::Settings,
    flags(no_refs_to_children),
    attrs(
        attr_default("window", &Settings::window, ayu::Tree::object()),
        attr_default("layout", &Settings::layout, ayu::Tree::object()),
        attr_default("render", &Settings::render, ayu::Tree::object()),
        attr_default("control", &Settings::control, ayu::Tree::object()),
        attr_default("files", &Settings::files, ayu::Tree::object()),
        attr_default("memory", &Settings::memory, ayu::Tree::object()),
        attr_default("mappings", &Settings::mappings, ayu::Tree::object()),
        attr("parent", &Settings::parent, optional)
    ),
    init<&Settings::canonicalize>()
)

#ifndef TAP_DISABLE_TESTS
#include "../dirt/ayu/resources/resource.h"
#include "../dirt/iri/iri.h"
#include "../dirt/tap/tap.h"

static tap::TestSet tests ("liv/settings", []{
    using namespace tap;

     // This is already covered by other tests here, but it's useful to isolate
     // this for performance testing.
    auto default_res = ayu::SharedResource("res:/liv/settings-default.ayu");
    auto settings_res = ayu::SharedResource("res:/liv/settings-template.ayu");
    is(default_res->state(), ayu::RS::Unloaded, "Default settings not loaded yet");
    doesnt_throw([&]{ ayu::load(settings_res); }, "Can load initial settings");
    is(default_res->state(), ayu::RS::Loaded,
        "Loading initial settings loads default settings"
    );
    const Settings* default_settings = default_res->ref();
    const Settings* settings = settings_res->ref();
    is(settings->parent, default_settings,
        "Settings linked properly to default settings"
    );
    done_testing();
});
#endif
