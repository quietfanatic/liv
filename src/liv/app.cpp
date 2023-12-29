#include "app.h"

#include <filesystem>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include "../dirt/iri/path.h"
#include "../dirt/ayu/resources/resource.h"
#include "book-source.h"
#include "book.h"
#include "memory.h"
#include "settings.h"

namespace liv {

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
                    current_book->view.window_size_changed({
                        event->window.data1,
                        event->window.data2
                    });
                    break;
                }
                case SDL_WINDOWEVENT_EXPOSED: {
                    current_book->view.need_draw = true;
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
                     // TODO: move the settings check to book.h or state.h
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
     // No more events?  Draw a book or do some background processing
    for (auto& book : self.books) {
        if (book->view.draw_if_needed()) return true;
    }
    bool need_remember = false;
    for (auto& book : self.books) {
         // This prioritizes earlier-numbered books.  Probably
         // doesn't matter though, since idle processing generally
         // happens in response to user input, and the user is
         // probably only interacting with one book.  And currently
         // we only have one book per process anyway.
        if (book->block.idle_processing(&*book, self.settings)) return true;
        need_remember |= book->need_remember;
    }
    if (need_remember) {
        Memory* memory = self.memory_res.ref();
        for (auto& book : self.books) {
            if (book->need_remember) {
                memory->remember_book(&*book);
                book->need_remember = false;
            }
        }
        ayu::save(self.memory_res);
        ayu::unload(self.memory_res);
        return true;
    }
    return false;
}

App::App () {
     // Load settings.
     // This doesn't really need to use AYU's resource system, but I need an app
     // to test it while I'm not developing games.
    settings_res = ayu::Resource("data:/settings.ayu");
    if (!ayu::source_exists(settings_res)) {
        fs::copy(
            ayu::resource_filename("res:/liv/settings-template.ayu"),
            ayu::resource_filename(settings_res)
        );
    }
    settings = settings_res.ref();
     // Don't load memory until until we need it.
    memory_res = ayu::Resource("data:/memory.ayu");
     // Set loop handlers
    loop.on_event = [this](SDL_Event* event){ on_event(*this, event); };
    loop.on_idle = [this](){ return on_idle(*this); };
}

App::~App () { }

static void add_book (App& self, std::unique_ptr<BookSource>&& src) {
    if (!ayu::source_exists(self.memory_res)) {
        self.memory_res.set_value(ayu::Dynamic::make<Memory>());
        ayu::save(self.memory_res);
    }
    auto& book = self.books.emplace_back(
        std::make_unique<Book>(&self, move(src), self.memory_res.ref())
    );
    ayu::unload(self.memory_res);

    self.books_by_window_id.emplace(
        glow::require_sdl(SDL_GetWindowID(book->view.window)),
        &*book
    );
}

void App::open_args (Slice<AnyString> args) {
    if (args.size() == 1) {
        if (fs::is_directory(args[0])) {
            open_folder(args[0]);
        }
        else open_file(args[0]);
    }
    else open_files(args);
}

void App::open_files (Slice<AnyString> filenames) {
    auto iris = UniqueArray<IRI>(filenames.size(), [=](usize i){
        return iri::from_fs_path(filenames[i]);
    });
    auto src = std::make_unique<BookSource>(
        settings, BookType::Misc, iris
    );
    add_book(*this, move(src));
}

void App::open_file (const AnyString& file) {
    auto loc = iri::from_fs_path(file);
    auto src = std::make_unique<BookSource>(
        settings, BookType::FileWithNeighbors, loc
    );
    add_book(*this, move(src));
}

void App::open_folder (const AnyString& folder) {
    auto loc = iri::from_fs_path(cat(folder, "/"));
    auto src = std::make_unique<BookSource>(
        settings, BookType::Folder, loc
    );
    add_book(*this, move(src));
}

void App::open_list (const AnyString& list_path) {
    constexpr IRI stdin_loc ("liv:stdin");
    auto loc = list_path == "-" ? stdin_loc : iri::from_fs_path(list_path);
    auto src = std::make_unique<BookSource>(
        settings, BookType::List, loc
    );
    add_book(*this, move(src));
}

void App::close_book (Book* book) {
    require(book);
    books_by_window_id.erase(
        glow::require_sdl(SDL_GetWindowID(book->view.window))
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

} using namespace liv;

#ifndef TAP_DISABLE_TESTS
#include <cstring>
#include <SDL2/SDL.h>
#include "../dirt/tap/tap.h"
#include "../dirt/glow/image.h"

static tap::TestSet tests ("liv/app", []{
    using namespace tap;

    fs::current_path(iri::to_fs_path(iri::program_location().without_filename()));

    App app;
     // TODO: Figure out how to get headless rendering working on nvidia drivers
    //app.hidden = true;
    doesnt_throw([&]{
        app.open_files({
            "/res/liv/test/image.png",
            "/res/liv/test/image2.png"
        });
    }, "App::open_files");
    auto window_id = glow::require_sdl(SDL_GetWindowID(app.books[0]->view.window));

    is(app.books[0]->state.get_page_number(), 1, "Book starts on page 1");

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

    is(app.books[0]->state.get_page_number(), 2, "Pressing right goes to next page");

    control::send_input_as_event(
        {.type = control::KEY, .code = SDLK_LEFT}, window_id
    );
    SDL_PushEvent(&quit_event);
    app.run();

    is(app.books[0]->state.get_page_number(), 1, "Pressing left goes to previous page");

    control::send_input_as_event(
        {.type = control::KEY, .ctrl = true, .code = SDLK_q}, window_id
    );
    app.run();
    pass("App stopped on Ctrl-Q");

    done_testing();
});

#endif
