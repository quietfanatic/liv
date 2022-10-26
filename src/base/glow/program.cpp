#include "program.h"

#include "../hacc/haccable-standard.h"
#include "gl.h"

using namespace std::literals;

namespace glow {

Shader::Shader (uint type) :
    id(type ? glCreateShader(type) : 0)
{ }

Shader::~Shader () {
    if (id) glDeleteShader(id);
}

void Shader::compile () {
    AA(id);
    glCompileShader(id);
    int status = 0; glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    int loglen = 0; glGetShaderiv(id, GL_INFO_LOG_LENGTH, &loglen);
    if (!status || loglen > 16) {
        auto info_log = std::string(loglen, 0);
        glGetShaderInfoLog(id, loglen, nullptr, info_log.data());
        throw X::ShaderCompileFailed(this, std::move(info_log));
    }
}

static Program* current_program = null;

Program::Program () : id(glCreateProgram()) { }

Program::~Program () {
    if (id) glDeleteProgram(id);
}

void Program::link () {
    AA(id);
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
        throw X::ProgramLinkFailed(this, std::move(info_log));
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
    hacc::dump(status);
    hacc::dump(info_log);
}

enum ShaderType { };

} using namespace glow;

HACCABLE(glow::ShaderType,
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

HACCABLE(glow::Shader,
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
        attr("source", mixed_funcs<String>(
            [](const Shader& v){
                AA(v.id);
                int len = 0;
                glGetShaderiv(v.id, GL_SHADER_SOURCE_LENGTH, &len);
                String r (len-1, 0);
                glGetShaderSource(v.id, len, null, r.data());
                return r;
            },
            [](Shader& v, const String& s){
                const char* src_p = s.c_str();
                int src_len = s.size();
                glShaderSource(v.id, 1, &src_p, &src_len);
            }
        ))
    )
)

HACCABLE(glow::Program,
    attrs(
        attr("shaders", &Program::shaders)
    ),
    init([](Program& v){ v.link(); })
)

HACCABLE(glow::X::ShaderCompileFailed,
    elems(
        elem(&X::ShaderCompileFailed::shader),
        elem(&X::ShaderCompileFailed::info_log)
    )
)

HACCABLE(glow::X::ProgramLinkFailed,
    elems(
        elem(&X::ProgramLinkFailed::program),
        elem(&X::ProgramLinkFailed::info_log)
    )
)
