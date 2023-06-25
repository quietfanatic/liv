#include "program.h"

#include "../ayu/describe.h"
#include "gl.h"

namespace glow {

Shader::Shader (uint type) : id(0) {
    if (type) {
        init();
        const_cast<uint&>(id) = glCreateShader(type);
    }
}

Shader::~Shader () {
    if (id) glDeleteShader(id);
}

void Shader::compile () {
    require(id);
    glCompileShader(id);
    int status = 0; glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    int loglen = 0; glGetShaderiv(id, GL_INFO_LOG_LENGTH, &loglen);
    if (!status || loglen > 16) {
        auto info_log = std::string(loglen, 0);
        glGetShaderInfoLog(id, loglen, nullptr, info_log.data());
        throw ayu::X<ShaderCompileFailed>(
            ayu::reference_to_location(this), move(info_log)
        );
    }
}

static Program* current_program = null;

Program::Program () {
    init();
    const_cast<uint&>(id) = glCreateProgram();
}

Program::~Program () {
    if (id) glDeleteProgram(id);
}

void Program::link () {
    require(id);
     // Detach old shaders
    int n_to_detach; glGetProgramiv(id, GL_ATTACHED_SHADERS, &n_to_detach);
    if (n_to_detach) {
        uint to_detach [n_to_detach];
        glGetAttachedShaders(id, n_to_detach, null, to_detach);
        for (auto s : to_detach) {
            glDetachShader(id, s);
        }
    }
     // Attach new shaders
    for (auto* s : shaders) {
        int status = 0; glGetShaderiv(s->id, GL_COMPILE_STATUS, &status);
        if (!status) s->compile();
        glAttachShader(id, s->id);
    }
     // Link
    Program_before_link();
    glLinkProgram(id);
    int status = 0; glGetProgramiv(id, GL_LINK_STATUS, &status);
    int loglen = 0; glGetProgramiv(id, GL_INFO_LOG_LENGTH, &loglen);
    if (!status || loglen > 16) {
        auto info_log = std::string(loglen, 0);
        glGetProgramInfoLog(id, loglen, nullptr, info_log.data());
        throw ayu::X<ProgramLinkFailed>(
            ayu::reference_to_location(this), move(info_log)
        );
    }
     // Extra
    if (current_program) current_program->unuse();
    glUseProgram(id);
    current_program = this;
    Program_after_link();
}

void Program::use () {
    if (current_program == this) return;
    if (current_program) {
        current_program->unuse();
    }
    glUseProgram(id);
    current_program = this;
    Program_after_use();
}

void Program::unuse () {
    if (current_program != this) return;
    Program_before_unuse();
    glUseProgram(0);
    current_program = null;
}

void Program::validate () {
    glValidateProgram(id);
    int status = 0; glGetProgramiv(id, GL_VALIDATE_STATUS, &status);
    int loglen = 0; glGetProgramiv(id, GL_INFO_LOG_LENGTH, &loglen);
    auto info_log = std::string(loglen, 0);
    glGetProgramInfoLog(id, loglen, nullptr, info_log.data());
    ayu::dump(status);
    ayu::dump(info_log);
}

enum ShaderType { };

} using namespace glow;

AYU_DESCRIBE(glow::ShaderType,
    values(
        value(0, ShaderType(0)),
        value("GL_COMPUTE_SHADER", ShaderType(GL_COMPUTE_SHADER)),
        value("GL_VERTEX_SHADER", ShaderType(GL_VERTEX_SHADER)),
        value("GL_TESS_CONTROL_SHADER", ShaderType(GL_TESS_CONTROL_SHADER)),
        value("GL_TESS_EVALUATION_SHADER", ShaderType(GL_TESS_EVALUATION_SHADER)),
        value("GL_GEOMETRY_SHADER", ShaderType(GL_GEOMETRY_SHADER)),
        value("GL_FRAGMENT_SHADER", ShaderType(GL_FRAGMENT_SHADER))
    )
)

AYU_DESCRIBE(glow::Shader,
    attrs(
        attr("type", value_funcs<ShaderType>(
            [](const Shader& v){
                if (v.id) {
                    int type = 0;
                    glGetShaderiv(v.id, GL_SHADER_TYPE, &type);
                    return ShaderType(type);
                }
                else return ShaderType(0);
            },
            [](Shader& v, ShaderType type){
                if (v.id) glDeleteShader(v.id);
                if (type) {
                    const_cast<uint&>(v.id) = glCreateShader(type);
                }
            }
        )),
        attr("source", mixed_funcs<std::string>(
            [](const Shader& v){
                require(v.id);
                int len = 0;
                glGetShaderiv(v.id, GL_SHADER_SOURCE_LENGTH, &len);
                std::string r (len-1, 0);
                glGetShaderSource(v.id, len, null, r.data());
                return r;
            },
            [](Shader& v, const std::string& s){
                const char* src_p = s.c_str();
                int src_len = s.size();
                glShaderSource(v.id, 1, &src_p, &src_len);
            }
        ))
    )
)

AYU_DESCRIBE(glow::Program,
    attrs(
        attr("shaders", &Program::shaders)
    ),
    init<&Program::link>()
)

AYU_DESCRIBE(glow::ShaderCompileFailed,
    delegate(base<glow::GlowError>()),
    elems(
        elem(&glow::ShaderCompileFailed::location),
        elem(&glow::ShaderCompileFailed::info_log)
    )
)

AYU_DESCRIBE(glow::ProgramLinkFailed,
    delegate(base<glow::GlowError>()),
    elems(
        elem(&glow::ProgramLinkFailed::location),
        elem(&glow::ProgramLinkFailed::info_log)
    )
)

#ifndef TAP_DISABLE_TESTS
#include "../ayu/resource.h"
#include "../geo/rect.h"
#include "../geo/vec.h"
#include "../tap/tap.h"
#include "../wind/window.h"
#include "colors.h"
#include "test-environment.h"

static tap::TestSet tests ("base/glow/program", []{
    using namespace tap;
    using namespace geo;
    TestEnvironment env;

    Program* program;
    doesnt_throw([&]{
        program = ayu::Resource("test:/test-program.ayu")["program"][1];
    }, "Can load program from ayu document");
    program->use();
    int u_screen_rect = glGetUniformLocation(*program, "u_screen_rect");
    isnt(u_screen_rect, -1, "Can get a uniform location");
    auto screen_rect = Rect{-0.5, -0.5, 0.5, 0.5};
    doesnt_throw([&]{
        glUniform1fv(u_screen_rect, 4, &screen_rect.l);
    }, "Can set uniform array");
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    doesnt_throw([&]{
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }, "glDrawArrays");

    UniqueArray<RGBA8> expected_pixels (area(env.size));
    for (int y = 0; y < env.size.y; y++)
    for (int x = 0; x < env.size.x; x++) {
        if (y >= env.size.y / 4 && y < env.size.y * 3 / 4
         && x >= env.size.x / 4 && x < env.size.x * 3 / 4) {
            expected_pixels[y*env.size.x+x] = RGBA8(30, 40, 50, 60);
        }
        else {
            expected_pixels[y*env.size.x+x] = RGBA8(0, 0, 0, 0);
        }
    }

    UniqueArray<RGBA8> got_pixels (area(env.size));
    glFinish();
    glReadPixels(0, 0, env.size.x, env.size.y, GL_RGBA, GL_UNSIGNED_BYTE, got_pixels.data());

    if (!is(got_pixels, expected_pixels, "Rendered correct image")) {
        diag(ayu::item_to_string(&got_pixels, ayu::COMPACT));
        diag(ayu::item_to_string(&expected_pixels, ayu::COMPACT));
    }

    done_testing();
});

#endif
