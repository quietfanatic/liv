#include "app.h"

#include <filesystem>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include "../base/ayu/parse.h"
#include "../base/ayu/resource.h"
#include "files.h"
#include "settings.h"

namespace app {

static Book* book_with_window_id (App& self, uint32 id) {
    auto iter = self.books_by_window_id.find(id);
    require(iter != self.books_by_window_id.end());
    return &*iter->second;
}

static void on_event (App& self, SDL_Event* event) {
    current_app = &self;
    switch (event->type) {
        case SDL_QUIT: self.stop(); break;
        case SDL_WINDOWEVENT: {
            current_book = book_with_window_id(self, event->window.windowID);
            switch (event->window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED: {
                    current_book->window_size_changed({
                        event->window.data1,
                        event->window.data2
                    });
                    break;
                }
                case SDL_WINDOWEVENT_EXPOSED: {
                    current_book->need_draw = true;
                    break;
                }
                case SDL_WINDOWEVENT_CLOSE: {
                    self.close_book(current_book);
                    current_book = null;
                    break;
                }
            }
            break;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            SDL_ShowCursor(SDL_DISABLE);
            current_book = book_with_window_id(self, event->key.windowID);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            SDL_ShowCursor(SDL_ENABLE);
            current_book = book_with_window_id(self, event->button.windowID);
            break;
        }
        case SDL_MOUSEMOTION: {
            SDL_ShowCursor(SDL_ENABLE);
            if (event->motion.state & SDL_BUTTON_RMASK) {
                current_book = book_with_window_id(
                    self, event->motion.windowID
                );
                if (current_book) {
                    current_book->drag(geo::Vec(
                        event->motion.xrel,
                        event->motion.yrel
                    ) * self.settings->get(&ControlSettings::drag_speed));
                }
            }
            break;
        }
         // TODO: Support wheel
        default: break;
    }
     // TODO: Move this waterfall to settings.h somehow
    for (auto& [input, action] : self.settings->mappings) {
        if (input_matches_event(input, event)) {
            if (action) action();
            goto done_mapping;
        }
    }
    for (auto& [input, action] : res_default_settings->mappings) {
        if (input_matches_event(input, event)) {
            if (action) action();
            goto done_mapping;
        }
    }
    for (auto& [input, action] : builtin_default_settings.mappings) {
        if (input_matches_event(input, event)) {
            if (action) action();
            goto done_mapping;
        }
    }
    done_mapping:
    current_book = null;
    current_app = null;
}

static bool on_idle (App& self) {
     // No more events?  Draw or do some background processing
    bool did_stuff = false;
    for (auto& book : self.books) {
        if (book->draw_if_needed()) {
            did_stuff = true;
        }
    }
    if (did_stuff) return true;
    for (auto& book : self.books) {
         // This prioritizes earlier-numbered books.  Probably
         // doesn't matter though, since idle processing generally
         // happens in response to user input, and the user is
         // probably only interacting with one book.  And currently
         // we only have one book per process anyway.
        if (book->idle_processing()) return true;
    }
    return false;
}

App::App () {
    ayu::Resource settings_ayu ("data:/settings.ayu");
    if (!ayu::source_exists(settings_ayu)) {
        fs::copy(
            ayu::resource_filename("res:/app/settings-template.ayu"),
            ayu::resource_filename(settings_ayu)
        );
    }
    settings = settings_ayu.ref();
    loop.on_event = [this](SDL_Event* event){ on_event(*this, event); };
    loop.on_idle = [this](){ return on_idle(*this); };
}

App::~App () { }

static void add_book (App& self, std::unique_ptr<Book>&& b) {
    auto& book = self.books.emplace_back(std::move(b));
    self.books_by_window_id.emplace(
        glow::require_sdl(SDL_GetWindowID(book->window)),
        &*book
    );
}

void App::open_files (std::vector<std::string>&& files) {
    add_book(*this, std::make_unique<Book>(
        *this, expand_files(settings, std::move(files))
    ));
}

void App::open_list (Str list_filename) {
    return open_files(read_list(list_filename));
}

void App::close_book (Book* book) {
    require(book);
    books_by_window_id.erase(
        glow::require_sdl(SDL_GetWindowID(book->window))
    );
    for (auto iter = books.begin(); iter != books.end(); iter++) {
        if (&**iter == book) {
            books.erase(iter);
            return;
        }
    }
    require(false);
}

void App::run () {
    loop.start();
}

void App::stop () {
    loop.stop();
}

App* current_app = null;
Book* current_book = null;

} using namespace app;

#ifndef TAP_DISABLE_TESTS
#include <cstring>
#include <SDL2/SDL.h>
#include "../base/tap/tap.h"
#include "../base/glow/image.h"

static tap::TestSet tests ("app/app", []{
    using namespace tap;

    char* base = glow::require_sdl(SDL_GetBasePath());
    std::string exe_folder = base;
    SDL_free(base);

    App app;
     // TODO: Figure out how to get headless rendering working on nvidia drivers
    //app.hidden = true;
    doesnt_throw([&]{
        app.open_files({
            ayu::cat(exe_folder, "/res/base/glow/test/image.png"sv),
            ayu::cat(exe_folder, "/res/base/glow/test/image2.png"sv)
        });
    }, "App::open_files");
    auto window_id = glow::require_sdl(SDL_GetWindowID(app.books[0]->window));

    is(app.books[0]->get_page_offset(), 1, "Book starts on page 1");

    SDL_Event quit_event;
    std::memset(&quit_event, 0, sizeof(quit_event));
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);
    app.run();
    pass("App stopped on SDL_QUIT message");

    control::send_input_as_event(
        {.type = control::KEY, .code = SDLK_RIGHT}, window_id
    );
    SDL_PushEvent(&quit_event);
    app.run();

    is(app.books[0]->get_page_offset(), 2, "Pressing right goes to next page");

    control::send_input_as_event(
        {.type = control::KEY, .code = SDLK_LEFT}, window_id
    );
    SDL_PushEvent(&quit_event);
    app.run();

    is(app.books[0]->get_page_offset(), 1, "Pressing left goes to previous page");

    control::send_input_as_event(
        {.type = control::KEY, .ctrl = true, .code = SDLK_q}, window_id
    );
    app.run();
    pass("App stopped on Ctrl-Q");

    done_testing();
});

#endif
