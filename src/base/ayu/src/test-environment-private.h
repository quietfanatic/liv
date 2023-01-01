#pragma once

#include <memory>
 // TODO don't rely on SDL for a non-gui library hahaha
#include <SDL2/SDL_filesystem.h>
#include "../../tap/tap.h"
#include "../document.h"
#include "../resource-scheme.h"

namespace ayu::test {
    struct TestResourceScheme : FileResourceScheme {
        using FileResourceScheme::FileResourceScheme;
        bool accepts_type (Type type) const override {
            return type == Type::CppType<Document>();
        }
    };
    struct TestEnvironment {
        std::unique_ptr<TestResourceScheme> trs;
        TestEnvironment () {
             // SDL does a whole lot of work to find this, which I cannot
             // reproduce.
            char* base = SDL_GetBasePath();
            trs = std::make_unique<test::TestResourceScheme>(
                "ayu-test", old_cat(base, "res/base/ayu/src/test")
            );
            SDL_free(base);
        }
    };
}
