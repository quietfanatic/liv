#include "window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

#include "../glow/gl.h"
#include "../ayu/describe.h"

namespace wind {

Window::Window (const GLAttributes& attrs) : Window(
    "",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    0, 0,
    SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN,
    attrs
) { }

Window::Window (
    const char* title, geo::IVec size,
    const GLAttributes& attrs
) : Window(
    title,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    size.x, size.y,
    SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN,
    attrs
) { }

Window::Window (
    const char* title, int x, int y, int w, int h, uint32 flags,
    const GLAttributes& attrs
) {
    glow::require_sdl(!SDL_InitSubSystem(SDL_INIT_VIDEO));
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, attrs.red);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, attrs.green);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, attrs.blue);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, attrs.alpha);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, attrs.depth);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, attrs.stencil);
    sdl_window = glow::require_sdl(SDL_CreateWindow(title, x, y, w, h, flags));
    if (flags & SDL_WINDOW_OPENGL) {
        gl_context = glow::require_sdl(SDL_GL_CreateContext(sdl_window));
        glow::require_sdl(!SDL_GL_SetSwapInterval(1));
    }
}

Window::~Window () {
     // Is this necessary?
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(sdl_window);
}

} using namespace wind;

AYU_DESCRIBE(wind::Window,
    attrs(
        attr("title", mixed_funcs<std::string>(
            [](const Window& window){
                return std::string(SDL_GetWindowTitle(window));
            },
            [](Window& window, const std::string& title){
                SDL_SetWindowTitle(window, title.c_str());
            }
        ), optional),
        attr("size", value_funcs<geo::IVec>(
            [](const Window& window){
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                return geo::IVec(w, h);
            },
            [](Window& window, geo::IVec size){
                SDL_SetWindowSize(window, size.x, size.y);
            }
        ), optional),
        attr("resizable", value_funcs<bool>(
            [](const Window& window) -> bool {
                return SDL_GetWindowFlags(window) & SDL_WINDOW_RESIZABLE;
            },
            [](Window& window, bool resizable){
                 // May not work if the window is fullscreen
                SDL_SetWindowResizable(window, resizable ? SDL_TRUE : SDL_FALSE);
            }
        ), optional),
        attr("fullscreen", value_funcs<bool>(
            [](const Window& window) -> bool {
                auto flags = SDL_GetWindowFlags(window);
                return flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP);
            },
            [](Window& window, bool fullscreen){
                SDL_SetWindowFullscreen(window,
                    fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
                );
            }
        ), optional),
         // Put this last so that everything else will be set first
        attr("hidden", value_funcs<bool>(
            [](const Window& window) -> bool {
                return SDL_GetWindowFlags(window) & SDL_WINDOW_HIDDEN;
            },
            [](Window& window, bool hidden){
                 // May not work if the window is fullscreen
                if (hidden) SDL_HideWindow(window);
                else SDL_ShowWindow(window);
            }
        ), optional)
    )
)
