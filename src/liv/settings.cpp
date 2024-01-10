#include "settings.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/ayu/resources/resource.h"

namespace liv {

const Settings builtin_default_settings = {
    .window = {
        .size = geo::IVec{720, 720},
        .fullscreen = false,
        .title = FormatList(
            FormatToken("["), FormatToken(FormatCommand::VisibleRange),
            FormatToken("/"), FormatToken(FormatCommand::PageCount),
            FormatToken("] "), FormatToken(FormatCommand::PageRelCwd),
            FormatToken(FormatCommand::IfZoomed, FormatList(
                FormatToken(" ("), FormatToken(FormatCommand::ZoomPercent),
                FormatToken("%)")
            ))
        ),
        .hidden = false,
        .automated_input = false,
    },
    .layout = {
        .spread_count = 1,
        .spread_direction = Direction::Right,
        .auto_zoom_mode = AutoZoomMode::Fit,
        .max_zoom = 32,
        .min_zoomed_size = 16,
        .reset_zoom_on_page_turn = true,
        .small_align = geo::Vec{0.5, 0.5},
        .large_align = geo::Vec{0.5, 0.5},
    },
    .render = {
        .interpolation_mode = InterpolationMode::SmartCubic,
        .window_background = Fill::Black,
        .transparency_background = Fill::White,
    },
    .control = {
        .drag_speed = 1,
    },
    .files = {
        .sort = SortMethod{
            SortCriterion::Natural, SortFlags::NotArgs | SortFlags::NotLists
        },
        .page_extensions = std::set<AnyString>{
            "bmp", "gif", "jfif", "jpe", "jpeg", "jpg",
            "png", "tif", "tiff", "xbm", "xpm", "webp",
        },
    },
    .memory = {
        .preload_ahead = 1,
        .preload_behind = 1,
        .page_cache_mb = 200,
        .trim_when_minimized = TrimMode::PageCache,
    },
    .mappings = { },
    .parent = null,
};

const Settings* app_settings () {
    static auto res = []{
        auto r = ayu::Resource(app_settings_location);
        if (!ayu::source_exists(r)) {
            fs::copy_file(
                ayu::resource_filename("res:/liv/settings-template.ayu"),
                ayu::resource_filename(r)
            );
        }
        return r;
    }();
    return res.ref();
}

void Settings::merge (Settings&& o) {
     // Is there a better solution than this?
#define LIV_MERGE(p) if (o.p) p = move(o.p);
    LIV_MERGE(window.size)
    LIV_MERGE(window.fullscreen)
    LIV_MERGE(window.title)
    LIV_MERGE(window.hidden)
    LIV_MERGE(window.automated_input)
    LIV_MERGE(layout.spread_count)
    LIV_MERGE(layout.spread_direction)
    LIV_MERGE(layout.auto_zoom_mode)
    LIV_MERGE(layout.max_zoom)
    LIV_MERGE(layout.min_zoomed_size)
    LIV_MERGE(layout.reset_zoom_on_page_turn)
    LIV_MERGE(layout.small_align)
    LIV_MERGE(layout.large_align)
    LIV_MERGE(render.interpolation_mode)
    LIV_MERGE(render.window_background)
    LIV_MERGE(render.transparency_background)
    LIV_MERGE(control.drag_speed)
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

const control::Statement* Settings::map_input (control::Input input) const {
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

AYU_DESCRIBE(liv::InterpolationMode,
    values(
        value("nearest", InterpolationMode::Nearest),
        value("linear", InterpolationMode::Linear),
        value("smoothed", InterpolationMode::Smoothed),
        value("cubic", InterpolationMode::Cubic),
        value("smart_cubic", InterpolationMode::SmartCubic)
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

AYU_DESCRIBE(liv::LayoutSettings,
    attrs(
        attr("spread_count", &LayoutSettings::spread_count, collapse_optional),
        attr("spread_direction", &LayoutSettings::spread_direction, collapse_optional),
        attr("auto_zoom_mode", &LayoutSettings::auto_zoom_mode, collapse_optional),
        attr("reset_zoom_on_page_turn", &LayoutSettings::reset_zoom_on_page_turn, collapse_optional),
        attr("max_zoom", &LayoutSettings::max_zoom, collapse_optional),
        attr("min_zoomed_size", &LayoutSettings::min_zoomed_size, collapse_optional),
        attr("small_align", &LayoutSettings::small_align, collapse_optional),
        attr("large_align", &LayoutSettings::large_align, collapse_optional)
    )
)
AYU_DESCRIBE(liv::RenderSettings,
    attrs(
        attr("interpolation_mode", &RenderSettings::interpolation_mode, collapse_optional),
        attr("window_background", &RenderSettings::window_background, collapse_optional),
        attr("transparency_background", &RenderSettings::transparency_background, collapse_optional)
    )
)

AYU_DESCRIBE(liv::WindowSettings,
    attrs(
        attr("size", &WindowSettings::size, collapse_optional),
        attr("fullscreen", &WindowSettings::fullscreen, collapse_optional),
        attr("title", &WindowSettings::title, collapse_optional)
    )
)

AYU_DESCRIBE(liv::FilesSettings,
    attrs(
        attr("sort", &FilesSettings::sort, collapse_optional),
        attr("page_extensions", &FilesSettings::page_extensions, collapse_optional)
    )
)

AYU_DESCRIBE(liv::ControlSettings,
    attrs(
        attr("drag_speed", &ControlSettings::drag_speed, collapse_optional)
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
    attrs(
        attr("window", &Settings::window, collapse_empty),
        attr("layout", &Settings::layout, collapse_empty),
        attr("render", &Settings::render, collapse_empty),
        attr("control", &Settings::control, collapse_empty),
        attr("files", &Settings::files, collapse_empty),
        attr("memory", &Settings::memory, collapse_empty),
        attr("mappings", &Settings::mappings, collapse_empty),
        attr("parent", &Settings::parent, optional)
    )
)

#ifndef TAP_DISABLE_TESTS
#include "../dirt/tap/tap.h"

static tap::TestSet tests ("liv/settings", []{
    using namespace tap;

     // This is already covered by other tests here, but it's useful to isolate
     // this for performance testing.
    auto default_res = ayu::Resource("res:/liv/settings-default.ayu");
    auto settings_res = ayu::Resource("res:/liv/settings-template.ayu");
    is(default_res.state(), ayu::UNLOADED, "Default settings not loaded yet");
    doesnt_throw([&]{ ayu::load(settings_res); }, "Can load initial settings");
    is(default_res.state(), ayu::LOADED,
        "Loading initial settings loads default settings"
    );
    const Settings* default_settings = default_res.ref();
    const Settings* settings = settings_res.ref();
    is(settings->parent, default_settings,
        "Settings linked properly to default settings"
    );
    done_testing();
});
#endif
