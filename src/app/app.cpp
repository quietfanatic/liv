#include "app.h"

#include <filesystem>
#include <unordered_set>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include "../base/ayu/resource.h"
#include "settings.h"

namespace fs = std::filesystem;

namespace app {

static Book* book_with_window_id (App& self, uint32 id) {
    auto iter = self.books_by_window_id.find(id);
    AA(iter != self.books_by_window_id.end());
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
                    ));
                }
            }
            break;
        }
         // TODO: Support wheel
        default: break;
    }
    for (auto& [input, action] : self.settings->mappings) {
        if (input_matches_event(input, event)) {
            action();
        }
    }
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

App::App () :
    settings(ayu::Resource("res:/app/settings.ayu").ref())
{
    loop.on_event = [this](SDL_Event* event){ on_event(*this, event); };
    loop.on_idle = [this](){ return on_idle(*this); };
}

App::~App () { }

static void add_book (App& self, std::unique_ptr<Book>&& b) {
    auto& book = self.books.emplace_back(std::move(b));
    self.books_by_window_id.emplace(
        AS(SDL_GetWindowID(book->window)),
        &*book
    );
}

void App::open_files (const std::vector<String>& files) {
     // Put supported extensions into a set for more speed
    std::unordered_set<Str> extensions;
    extensions.reserve(settings->files.supported_extensions.size());
    for (auto& ext : settings->files.supported_extensions) {
        extensions.emplace(ext);
    }

    Str folder;
    if (files.size() == 1) {
        if (fs::is_directory(files[0])) {
            folder = files[0];
        }
        else if (fs::exists(files[0])) {
            auto folder_p = fs::path(files[0]).remove_filename();
            std::u8string folder_u8 = folder_p.u8string();
            std::string& folder = reinterpret_cast<std::string&>(folder_u8);
            std::vector<String> real_files;
            for (auto& entry : fs::directory_iterator(folder_p)) {
                std::u8string u8name = entry.path().u8string();
                std::string& name = reinterpret_cast<std::string&>(u8name);
                Str extension;
                usize dotpos = name.rfind('.');
                if (dotpos != std::string::npos) {
                    extension = Str(&name[dotpos+1], name.size() - dotpos - 1);
                }
                if (!extensions.count(extension)) continue;
                real_files.emplace_back(std::move(name));
            }
            auto book = std::make_unique<Book>(*this, std::move(real_files), std::move(folder));
            book->set_page(book->get_page_no_with_filename(files[0]));
            add_book(*this, std::move(book));
            return;
        }
    }
    std::vector<String> real_files;
    for (auto& file : files) {
        if (fs::is_directory(file)) {
            for (auto& entry : fs::recursive_directory_iterator(file)) {
                std::u8string u8name = entry.path().u8string();
                std::string& name = reinterpret_cast<std::string&>(u8name);
                Str extension;
                usize dotpos = name.rfind('.');
                if (dotpos != std::string::npos) {
                    extension = Str(&name[dotpos+1], name.size() - dotpos - 1);
                }
                if (!extensions.count(extension)) continue;
                real_files.emplace_back(std::move(name));
            }
        }
        else real_files.emplace_back(file);
    }
    add_book(*this, std::make_unique<Book>(*this, std::move(real_files), String(folder)));
}

void App::close_book (Book* book) {
    AA(book);
    for (auto iter = books.begin(); iter != books.end(); iter++) {
        if (&**iter == book) {
            books.erase(iter);
            return;
        }
    }
    AA(false);
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

    char* base = AS(SDL_GetBasePath());
    String exe_folder = base;
    SDL_free(base);

    App app;
    app.hidden = true;
    doesnt_throw([&]{
        app.open_files({
            exe_folder + "/res/base/glow/test/image.png"sv,
            exe_folder + "/res/base/glow/test/image2.png"sv
        });
    }, "App::open_files");
    auto window_id = AS(SDL_GetWindowID(app.books[0]->window));

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
