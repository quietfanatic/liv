#pragma once

#include "common.h"

namespace glow {
    void register_gl_function (void*, const char*);
    void init_gl_functions ();

    void throw_on_glGetError (
        const char* gl_function,
        const char* caller,
        const char* filename,
        uint line
    );

    namespace X {
        struct GLError : GlowError {
            uint error_code;
            std::string gl_function;
            std::string caller;
            std::string filename;
            uint line;
            GLError (uint e, const char* g, const char* c, const char* f, uint l) :
                error_code(e), gl_function(g), caller(c), filename(f), line(l)
            { }
        };
    }
}

 // Build GL API

#define DECLARE_GL_FUNCTION(name, Ret, params) \
template <int = 0> \
Ret(APIENTRY* p_##name )params noexcept = \
    (glow::register_gl_function(&p_##name<>, #name), nullptr);

#ifdef NDEBUG

 // TODO: change to not require p_
#define USE_GL_FUNCTION(p_name) p_name<>

#else
 // For debug, check glGetError after every call.

#include <type_traits>

#ifndef APIENTRY
#ifdef _WIN32
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

namespace glow {
    template <class Ret, class... Args>
    auto checked_gl_function(
        Ret(APIENTRY* p )(Args...),
        const char* fname,
        const char* function,
        const char* filename,
        unsigned line
    ) {
         // Bad hack: Add 2 to fname to remove the p_
        return [=](Args... args) -> Ret {
            if constexpr (std::is_void_v<Ret>) {
                p(args...);
                throw_on_glGetError(fname + 2, function, filename, line);
            }
            else {
                Ret r = p(args...);
                throw_on_glGetError(fname + 2, function, filename, line);
                return r;
            }
        };
    }
}

#define USE_GL_FUNCTION(p_name) glow::checked_gl_function(p_name<>, #p_name, __FUNCTION__, __FILE__, __LINE__)

#endif

#include "gl_api.h"
