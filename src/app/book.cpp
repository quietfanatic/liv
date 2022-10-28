#include "book.h"

#include <filesystem>
#include <SDL2/SDL_video.h>
#include "../base/glow/gl.h"
#include "page.h"

using namespace geo;
namespace fs = std::filesystem;

namespace app {

Book::Book (Str folder) : folder(folder) {
    AA(false);
}
Book::Book (const std::vector<String>& filenames) :
    window{"Image Viewer", {640, 480}, true}
{
    window.open();
    glow::init();
    pages.reserve(filenames.size());
    for (auto& filename : filenames) {
        pages.emplace_back(std::make_unique<Page>(filename));
    }
}
Book::~Book () { }

void Book::draw () {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (current_page >= 0 && usize(current_page) < pages.size()) {
        pages[current_page]->draw(Rect(-1, -1, 1, 1));
    }
    SDL_GL_SwapWindow(window.sdl_window);
}

} // namespace app
