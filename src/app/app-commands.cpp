#include "app-commands.h"

#include "app.h"
#include "view.h"

namespace app::command {

static void prev_ () {
    if (current_book) current_book->prev();
}
Command prev (prev_, "prev", "Go to previous page or pages");

static void next_ () {
    if (current_book) current_book->next();
}
Command next (next_, "next", "Go to next page or pages");

static void quit_ () {
    if (current_app) current_app->stop();
}
Command quit (quit_, "quit", "Quit application");

static void fit_mode_ (FitMode mode) {
    if (current_book) current_book->view.fit_mode = mode;
}
Command fit_mode (fit_mode_, "fit_mode", "Set fit mode: fit, stretch, or manual");

} // namespace app::command
