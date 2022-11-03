#pragma once

#include "common.h"
#include "../ayu/path.h"
#include "../ayu/serialize.h"

namespace glow {

struct Shader {
    explicit Shader (uint type = 0);
    Shader (Shader&& o) : id(o.id) { const_cast<uint&>(o.id) = 0; }
    ~Shader ();

    void compile ();

    const uint id = 0;
    operator uint () const { return id; }
};

struct Program {
    std::vector<Shader*> shaders;

    virtual void Program_before_link () { }
    virtual void Program_after_link () { }
    virtual void Program_after_use () { }
    virtual void Program_before_unuse () { }

    Program ();
    Program (Program&& o) : id(o.id) { const_cast<uint&>(o.id) = 0; }
    ~Program ();

    void link ();
    void use ();
    void unuse ();
     // For render debugging
    void validate ();

    const uint id = 0;
    operator uint () const { return id; }
};

namespace X {
    struct ShaderCompileFailed : GlowError {
        ayu::Path path;
        std::string info_log;
        ShaderCompileFailed (Shader* s, std::string&& l) :
            path(ayu::reference_to_path(s)), info_log(l)
        { }
    };
    struct ProgramLinkFailed : GlowError {
        ayu::Path path;
        std::string info_log;
        ProgramLinkFailed (Program* p, std::string&& l) :
            path(ayu::reference_to_path(p)), info_log(l)
        { }
    };
}

} // namespace glow
