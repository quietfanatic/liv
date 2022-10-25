#include "loop.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>

#include "../hacc/haccable.h"

namespace wind {

void Loop::start () {
    double lag = 0;
    uint32 last_ticks = SDL_GetTicks();
    while (!stop_requested) {
         // SDL event loop
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    stop(); break;
                }
                case SDL_KEYDOWN: {
                    switch (event.key.keysym.scancode) {
                        case 0x29: { // Escape
                            stop(); break;
                        }
                        default: break;
                    }
                    break;
                }
                case SDL_WINDOWEVENT: {
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        // TODO
                        //size = {event.window.data1, event.window.data2};
                    }
                    break;
                }
                default: break;
            }
        }
         // Run step
        step();
         // Calculate timing
        lag -= 1/fps;
        if (lag > max_lag_tolerance/fps) {
             // Really bad lag!  Is step() taking too long?
            lag = 1/fps;
        }
        if (lag > (1 + min_lag_tolerance)/fps) {
             // Drop frame
        }
        else {
             // Tolerate tiny amount of lag
            if (lag > 1/fps) lag = 1/fps;
             // Render
            draw();
        }
         // Delay if we need to.
         // TODO: Is this the best place to put this?
        uint32 new_ticks = SDL_GetTicks();
        lag += (new_ticks - last_ticks) / 1000.0;
        last_ticks = new_ticks;
        if (lag < 0) {
            SDL_Delay(-lag * 1000);
        }
    }
}

void Loop::stop () {
    stop_requested = true;
}

} using namespace wind;

HACCABLE(wind::Loop,
    attrs(
        attr("fps", &Loop::fps, optional),
        attr("min_lag_tolerance", &Loop::min_lag_tolerance, optional),
        attr("max_lag_tolerance", &Loop::max_lag_tolerance, optional)
    )
)
