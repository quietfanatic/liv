#include "window.h"

#include <unordered_map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

#include "../hacc/haccable.h"

namespace wind {

 // TODO: unnecessarily heavyweight?
static std::unordered_map<uint32, Window*> open_windows;

void Window::update () {
    SDL_SetWindowTitle(sdl_window, title.c_str());
    SDL_SetWindowSize(sdl_window, size.x, size.y);
    SDL_SetWindowResizable(sdl_window, resizable ? SDL_TRUE : SDL_FALSE);
    if (hidden) SDL_HideWindow(sdl_window);
    else SDL_ShowWindow(sdl_window);
}

void Window::open () {
    if (sdl_window) {
        update();
        return;
    }
    AS(!SDL_InitSubSystem(SDL_INIT_VIDEO));
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);  // Temporary for testing
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    sdl_window = AS(SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        size.x,
        size.y,
        SDL_WINDOW_OPENGL | (hidden ? SDL_WINDOW_HIDDEN : 0)
    ));
    gl_context = AS(SDL_GL_CreateContext(sdl_window));
    AS(!SDL_GL_SetSwapInterval(1));
    uint32 id = AS(SDL_GetWindowID(sdl_window));
    auto [iter, emplaced] = open_windows.emplace(id, this);
    AA(emplaced);
}

void Window::close () {
    if (gl_context) {
         // Is this necessary?
        SDL_GL_DeleteContext(gl_context);
    }
    if (sdl_window) {
        uint32 id = AS(SDL_GetWindowID(sdl_window));
        AA(open_windows.erase(id));
        SDL_DestroyWindow(sdl_window);
    }
}

Window::~Window () {
    close();
}

bool process_window_event (SDL_Event* event) {
    uint32 id = 0;
    switch (event->type) {
        case SDL_WINDOWEVENT:
        case SDL_SYSWMEVENT: id = event->window.windowID; break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: id = event->key.windowID; break;
        case SDL_TEXTEDITING: id = event->edit.windowID; break;
        case SDL_TEXTINPUT: id = event->text.windowID; break;
//        case SDL_TEXTINPUT_EXT: id = event->editExt.windowID; break;
        case SDL_MOUSEMOTION: id = event->motion.windowID; break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: id = event->button.windowID; break;
        case SDL_MOUSEWHEEL: id = event->wheel.windowID; break;
        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
        case SDL_FINGERMOTION: id = event->tfinger.windowID; break;
        case SDL_DROPFILE:
        case SDL_DROPTEXT:
        case SDL_DROPBEGIN:
        case SDL_DROPCOMPLETE: id = event->drop.windowID; break;
        case SDL_USEREVENT: id = event->user.windowID; break;
    }
    auto iter = open_windows.find(id);
    if (iter != open_windows.end()) {
        auto on_event = iter->second->on_event;
        return on_event && on_event(event);
    }
    return false;
}

} using namespace wind;

HACCABLE(wind::Window,
    attrs(
        attr("title", &Window::title, optional),
        attr("size", &Window::size, optional),
        attr("resizable", &Window::resizable, optional),
        attr("hidden", &Window::hidden, optional)
    )
)
