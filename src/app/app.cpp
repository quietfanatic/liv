#include "app.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>

namespace app {

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
        switch (event.type) {
            case SDL_QUIT: return;
            case SDL_KEYDOWN: {
                Book* book = book_with_window_id(*this, event.key.windowID);
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: return;
                    case SDLK_LEFT: {
                        book->prev();
                        break;
                    }
                    case SDLK_RIGHT: {
                        book->next();
                        break;
                    }
                }
            }
            default: break;
        }
    }
}

void App::stop () {
    stop_requested = true;
}

} using namespace app;
