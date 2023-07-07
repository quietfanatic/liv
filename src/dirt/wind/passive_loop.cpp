#include "passive_loop.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>

#include "../ayu/describe.h"

namespace wind {

static void default_on_event (PassiveLoop& self, SDL_Event* event) {
    switch (event->type) {
        case SDL_QUIT: self.stop(); break;
        case SDL_KEYDOWN: {
            if (event->key.keysym.sym == SDLK_ESCAPE) self.stop();
            break;
        }
        default: break;
    }
}

void PassiveLoop::start () {
    stop_requested = false;
    for (;;) {
        SDL_Event event;
        SDL_PumpEvents();
        if (SDL_HasEvents(SDL_FIRSTEVENT, SDL_LASTEVENT)) {
            SDL_PollEvent(&event);
            if (on_event) on_event(&event);
            else default_on_event(*this, &event);
            if (stop_requested) return;
        }
        else {
            if (!on_idle || !on_idle()) {
                SDL_WaitEvent(null);
            }
        }
    }
}

void PassiveLoop::stop () {
    stop_requested = true;
}

} using namespace wind;

AYU_DESCRIBE(wind::PassiveLoop,
    attrs()
)
