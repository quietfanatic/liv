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

//TEMP
#include "../dirt/uni/io.h"

namespace liv {

static Book* book_with_window_id (App& self, uint32 id) {
    auto iter = self.books_by_window_id.find(id);
    require(iter != self.books_by_window_id.end());
    return &*iter->second;
}

static void on_event (App& self, SDL_Event* event) {
     // TODO: Move book-specific stuff to Book
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
                case SDL_WINDOWEVENT_FOCUS_GAINED: {
                    self.last_focused = event->window.timestamp;
                    break;
                }
            }
            break;
        }
        case SDL_KEYDOWN: {
             // There's a bug where if we gain focus by another window being
             // closed with a keystroke, the keystroke gets sent to us.  I don't
             // know if this is a bug in SDL or the window manager, but the
             // workaround is pretty simple: disable keyboard input right after
             // gaining focus.  In my testing, the difference is always either 0
             // or 1 ms, so we'll go up to 3 in case the computer is slow for
             // some reason.  This is still faster than 1 video frame and faster
             // than the typical input device polling rate (10ms).
            if (!self.automated_input &&
                event->key.timestamp - self.last_focused <= 3
            ) {
                return;
            }
            else [[fallthrough]];
        }
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
                    ) * current_book->state.settings->get(&ControlSettings::drag_speed));
                }
            }
            break;
        }
         // TODO: Support wheel
        default: break;
    }
    if (current_book) {
        auto action = current_book->state.settings->map_event(event);
        if (action && *action) (*action)();
    }
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
        if (book->need_memorize) {
            memorize_book(&*book);
            book->need_memorize = false;
            return true;
        }
    }
    return false;
}

App::App () {
     // Set loop handlers
    loop.on_event = [this](SDL_Event* event){ on_event(*this, event); };
    loop.on_idle = [this](){ return on_idle(*this); };
}

App::~App () { }

static void add_book (
    App& self, std::unique_ptr<BookSource> src,
    std::unique_ptr<Settings> settings
) {
    auto& book = self.books.emplace_back(
        std::make_unique<Book>(&self, move(src), move(settings))
    );
    remember_book(&*book);

    self.books_by_window_id.emplace(
        glow::require_sdl(SDL_GetWindowID(book->view.window)),
        &*book
    );
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
    auto src = std::make_unique<BookSource>(
        *settings, BookType::Misc, iris
    );
    add_book(*this, move(src), move(settings));
}

void App::open_file (
    const AnyString& file, std::unique_ptr<Settings> settings
) {
    auto loc = iri::from_fs_path(file);
    auto src = std::make_unique<BookSource>(
        *settings, BookType::FileWithNeighbors, loc
    );
    add_book(*this, move(src), move(settings));
}

void App::open_folder (
    const AnyString& folder, std::unique_ptr<Settings> settings
) {
    auto loc = iri::from_fs_path(cat(folder, "/"));
    auto src = std::make_unique<BookSource>(
        *settings, BookType::Folder, loc
    );
    add_book(*this, move(src), move(settings));
}

void App::open_list (
    const AnyString& list_path, std::unique_ptr<Settings> settings
) {
    constexpr IRI stdin_loc ("liv:stdin");
    auto loc = list_path == "-" ? stdin_loc : iri::from_fs_path(list_path);
    auto src = std::make_unique<BookSource>(
        *settings, BookType::List, loc
    );
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
    settings->parent = app_settings();
    App app;
     // TODO: Figure out how to get headless rendering working on nvidia drivers
    //app.hidden = true;
    app.automated_input = true;
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
