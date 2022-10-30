#include "app.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include "../base/ayu/resource.h"
#include "settings.h"

namespace app {

App::App () :
    settings(hacc::Resource("/app/settings.ayu").ref())
{ }

App::~App () { }

static void add_book (App& self, std::unique_ptr<Book>&& b) {
    auto& book = self.books.emplace_back(std::move(b));
    self.books_by_window_id.emplace(
        AS(SDL_GetWindowID(book->window.sdl_window)),
        &*book
    );
    book->draw();
}

static Book* book_with_window_id (App& self, uint32 id) {
    auto iter = self.books_by_window_id.find(id);
    AA(iter != self.books_by_window_id.end());
    return &*iter->second;
}

void App::open_folder (Str folder) {
    add_book(*this, std::make_unique<Book>(*this, folder));
}
void App::open_files (const std::vector<String>& files) {
    add_book(*this, std::make_unique<Book>(*this, files));
}

void App::run () {
    stop_requested = false;
    while (!stop_requested) {
        SDL_Event event;
        AS(SDL_WaitEvent(&event));
        current_app = this;
        switch (event.type) {
            case SDL_QUIT: stop(); break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                current_book = book_with_window_id(*this, event.key.windowID);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                current_book = book_with_window_id(*this, event.button.windowID);
                break;
            default: break;
        }
        for (auto& [input, action] : settings->mappings) {
            if (input_matches_event(input, &event)) {
                action();
            }
        }
        current_book = null;
        current_app = null;
    }
}

void App::stop () {
    stop_requested = true;
}

App* current_app = null;
Book* current_book = null;

} using namespace app;

#ifndef TAP_DISABLE_TESTS
#include <cstring>
#include "../base/tap/tap.h"
#include "../base/glow/image.h"

static tap::TestSet tests ("app/app", []{
    using namespace tap;
    App app;
    app.hidden = true;
    doesnt_throw([&]{
        app.open_files({
            hacc::file_resource_root() + "/base/glow/test/image.png"sv,
            hacc::file_resource_root() + "/base/glow/test/image2.png"sv
        });
    }, "App::open_files");
    auto window_id = AS(SDL_GetWindowID(app.books[0]->window.sdl_window));

    is(app.books[0]->current_page_no, 1, "Book starts on page 1");

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

    is(app.books[0]->current_page_no, 2, "Pressing right goes to next page");

    control::send_input_as_event(
        {.type = control::KEY, .code = SDLK_LEFT}, window_id
    );
    SDL_PushEvent(&quit_event);
    app.run();

    is(app.books[0]->current_page_no, 1, "Pressing left goes to previous page");

    control::send_input_as_event(
        {.type = control::KEY, .ctrl = true, .code = SDLK_q}, window_id
    );
    app.run();
    pass("App stopped on Ctrl-Q");

    done_testing();
});

#endif
