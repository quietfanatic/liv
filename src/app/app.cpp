#include "app.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include "../base/hacc/resource.h"
#include "settings.h"

namespace app {

App::App () :
    settings(hacc::Resource("/app/settings.hacc").ref())
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
        Book* book = null;
        switch (event.type) {
            case SDL_QUIT: stop(); break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                book = book_with_window_id(*this, event.key.windowID);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                book = book_with_window_id(*this, event.button.windowID);
                break;
            default: break;
        }
        if (input_matches_event(settings->prev, &event)) {
            book->prev();
        }
        else if (input_matches_event(settings->next, &event)) {
            book->next();
        }
        else if (input_matches_event(settings->quit, &event)) {
            stop();
        }
    }
}

void App::stop () {
    stop_requested = true;
}

} using namespace app;
