#include "window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

#include "../glow/gl.h"
#include "../ayu/describe.h"

namespace wind {

void Window::update () {
    SDL_SetWindowTitle(sdl_window, title.c_str());
    SDL_SetWindowSize(sdl_window, size.x, size.y);
    SDL_SetWindowResizable(sdl_window, resizable ? SDL_TRUE : SDL_FALSE);
    if (hidden) SDL_HideWindow(sdl_window);
    else SDL_ShowWindow(sdl_window);
}

 // TODO: Supposedly this may be called on a different thread.
 // This is probably dangerous?
static int handle_resize (void* userdata, SDL_Event* event) {
    if (event->type != SDL_WINDOWEVENT) return 0;
    if (event->window.event != SDL_WINDOWEVENT_SIZE_CHANGED) return 0;
    auto window = reinterpret_cast<Window*>(userdata);
    window->size = {event->window.data1, event->window.data2};
    glViewport(0, 0, window->size.x, window->size.y);
    return 0;
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
    SDL_SetWindowResizable(sdl_window, resizable ? SDL_TRUE : SDL_FALSE);
    SDL_AddEventWatch(&handle_resize, this);
    gl_context = AS(SDL_GL_CreateContext(sdl_window));
    AS(!SDL_GL_SetSwapInterval(1));
}

void Window::close () {
    if (gl_context) {
         // Is this necessary?
        SDL_GL_DeleteContext(gl_context);
    }
    if (sdl_window) {
        SDL_DestroyWindow(sdl_window);
    }
}

Window::~Window () {
    close();
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
