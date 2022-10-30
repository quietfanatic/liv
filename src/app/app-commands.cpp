#include "app-commands.h"

#include "app.h"

namespace app::command {

static void prev_ () {
    if (current_book) current_book->prev();
}
Command prev (prev_, "prev", "Go to previous page or pages");
 // () Go to next page(s)
static void next_ () {
    if (current_book) current_book->next();
}
Command next (next_, "next", "Go to next page or pages");
 // () Quit app
static void quit_ () {
    if (current_app) current_app->stop();
}
Command quit (quit_, "quit", "Quit application");

} // namespace app::command
