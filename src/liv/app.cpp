#include "app.h"

#include <filesystem>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include "../dirt/iri/path.h"
#include "../dirt/ayu/resources/resource.h"
#include "book-source.h"
#include "book.h"
#include "mark.h"
#include "settings.h"

//TEMP
#include "../dirt/uni/io.h"

namespace liv {

static Book* book_with_window_id (App& self, uint32 id) {
    auto iter = self.books_by_window_id.find(id);
    require(iter != self.books_by_window_id.end());
    return &*iter->second;
}

static void on_event (App& self, SDL_Event* e) {
    current_app = &self;
    switch (e->type) {
        case SDL_QUIT: self.stop(); break;
        case SDL_WINDOWEVENT: {
            current_book = book_with_window_id(self, e->window.windowID);
            switch (e->window.event) {
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
            current_book = book_with_window_id(self, e->key.windowID);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            SDL_ShowCursor(SDL_ENABLE);
            current_book = book_with_window_id(self, e->button.windowID);
            break;
        }
        case SDL_MOUSEMOTION: {
            SDL_ShowCursor(SDL_ENABLE);
            current_book = book_with_window_id(self, e->motion.windowID);
            break;
        }
         // TODO: Support wheel
        default: break;
    }
    if (current_book) current_book->on_event(e);
    current_book = null;
    current_app = null;
}

static bool on_idle (App& self) {
     // No more events?  Draw a book or do some background processing
    for (auto& book : self.books) {
        if (book->view.draw_if_needed()) return true;
    }
    for (auto& book : self.books) {
         // This prioritizes earlier-numbered books.  Probably
         // doesn't matter though, since idle processing generally
         // happens in response to user input, and the user is
         // probably only interacting with one book.  And currently
         // we only have one book per process anyway.
        if (book->block.idle_processing(&*book, *book->state.settings)) {
            return true;
        }
        if (book->need_mark) {
            book->need_mark = false;
            save_mark(*book);
            return true;
        }
    }
    return false;
}

App::App () : loop{
    .on_event = [this](SDL_Event* event){ on_event(*this, event); },
    .on_idle = [this](){ return on_idle(*this); },
} { }

App::~App () { }

static void add_book (
    App& self, BookSource&& src,
    std::unique_ptr<Settings> settings
) {
    auto book = load_mark(src, *settings);
    if (!book) {
         // By default parent the settings to the app settings.  We need a
         // better way of making this happen.
        if (settings->parent == &builtin_default_settings) {
            settings->parent = app_settings();
        }
        PageBlock block (src, *settings);
        BookState state (move(settings));
        if (src.type == BookType::FileWithNeighbors) {
            expect(src.locations.size() == 1);
            int32 start = block.find(src.locations[0]);
            if (start >= 0) state.page_offset = start;
        }
        book = std::make_unique<Book>(move(src), move(block), move(state));
    }
    self.books_by_window_id.emplace(
        glow::require_sdl(SDL_GetWindowID(book->view.window)),
        &*book
    );
    self.books.emplace_back(move(book));
}

void App::open_args (
    Slice<AnyString> args, std::unique_ptr<Settings> settings
) {
    if (args.size() == 1) {
        if (fs::is_directory(args[0])) {
            open_folder(args[0], move(settings));
        }
        else open_file(args[0], move(settings));
    }
    else open_files(args, move(settings));
}

void App::open_files (
    Slice<AnyString> filenames, std::unique_ptr<Settings> settings
) {
    auto iris = UniqueArray<IRI>(filenames.size(), [=](usize i){
        return iri::from_fs_path(filenames[i]);
    });
    auto src = BookSource(BookType::Misc, iris);
    add_book(*this, move(src), move(settings));
}

void App::open_file (
    const AnyString& file, std::unique_ptr<Settings> settings
) {
    auto loc = iri::from_fs_path(file);
    auto src = BookSource(BookType::FileWithNeighbors, Slice<IRI>{loc});
    add_book(*this, move(src), move(settings));
}

void App::open_folder (
    const AnyString& folder, std::unique_ptr<Settings> settings
) {
    auto loc = iri::from_fs_path(cat(folder, "/"));
    auto src = BookSource(BookType::Folder, Slice<IRI>{loc});
    add_book(*this, move(src), move(settings));
}

void App::open_list (
    const AnyString& list_path, std::unique_ptr<Settings> settings
) {
    constexpr IRI stdin_loc ("liv:stdin");
    auto loc = list_path == "-" ? stdin_loc : iri::from_fs_path(list_path);
    auto src = BookSource(BookType::List, Slice<IRI>{loc});
    add_book(*this, move(src), move(settings));
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

    fs::current_path(iri::to_fs_path(iri::program_location().chop_filename()));

    auto settings = std::make_unique<Settings>();
    settings->window.size = {{120, 120}};
     // TODO: Figure out how to get headless rendering working on nvidia drivers
    //settings->window.hidden = true;
    settings->window.automated_input = true;
    settings->parent = app_settings();
    App app;
    doesnt_throw([&]{
        app.open_files({
            "/res/liv/test/image.png",
            "/res/liv/test/image2.png"
        }, move(settings));
    }, "App::open_files");
    auto window_id = glow::require_sdl(SDL_GetWindowID(app.books[0]->view.window));

    is(app.books[0]->state.page_offset, 0, "Book starts on page 1");

    SDL_Event quit_event;
    std::memset(&quit_event, 0, sizeof(quit_event));
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);
    app.run();
    pass("App stopped on SDL_QUIT message");

    control::send_input_as_event(
        {.type = control::InputType::Key, .code = SDLK_RIGHT}, window_id
    );
    SDL_PushEvent(&quit_event);
    app.run();

    is(app.books[0]->state.page_offset, 1, "Pressing right goes to next page");

    control::send_input_as_event(
        {.type = control::InputType::Key, .code = SDLK_LEFT}, window_id
    );
    SDL_PushEvent(&quit_event);
    app.run();

    is(app.books[0]->state.page_offset, 0, "Pressing left goes to previous page");

    control::send_input_as_event(
        {.type = control::InputType::Key, .flags = control::InputFlags::Ctrl, .code = SDLK_q}, window_id
    );
    app.run();
    pass("App stopped on Ctrl-Q");

    done_testing();
});

#endif
