// A haccable function type, that can be used to make a non-turing-complete
// imperative DSL.

#pragma once

#include <utility>
#include "../hacc/dynamic.h" 
 // For tuple haccability
#include "../hacc/haccable-standard.h"
#include "../hacc/type.h"
#include "../uni/common.h"
#include "command-template-utils.h"

namespace control {

template <class F>
using Function = F;

 // This is how you define new commands.  Make static objects of these.
struct Command {
    Function<void(void*, void*)>* wrapper;
    void* function;
    String name;
    String description;
    usize required_arg_count;
    Function<hacc::Type()>* args_type;
    Function<std::vector<hacc::Type>()>* arg_types;

    template <class... Args>
    CE Command (
        Function<void(Args...)> f,
        Str name, Str desc, usize req = sizeof...(Args)
    ) :
        wrapper(
            CommandWrapper<Args...>::get_unwrap(std::index_sequence_for<Args...>{})
        ),
        function(*reinterpret_cast<void**>(&f)),
        name(name), description(desc), required_arg_count(req),
        args_type([]{
            return hacc::Type::CppType<StatementStorage<Args...>>();
        }),
        arg_types([]{
            static std::vector<hacc::Type> r = {hacc::Type::CppType<Args>()...};
            return r;
        })
    {
         // Make sure we aren't on a really weird architecture.
        static_assert(sizeof(decltype(f)) == sizeof(void*));
        register_command();
    }
  private:
    void register_command () const;
};

 // Returns nullptr if not found
const Command* lookup_command (Str name);
 // Throws X::CommandNotFound if not found
const Command* require_command (Str name);

 // The structure you create to use a command.  You can create this manually,
 // but it doesn't support optional arguments unless you deserialize from hacc.
struct Statement {
    const Command* command = null;
    hacc::Dynamic args;  // Type must be command->args_type (std::tuple)

    CE Statement() { }
    Statement (Command* c, hacc::Dynamic&& a);
    template <class... Args>
    Statement (Command* c, Args... args) :
        Statement(c, hacc::Dynamic(StatementStorage<Args...>(std::forward<Args>(args)...)))
    { }
    template <class... Args>
    Statement (Str name, Args... args) :
        Statement(require_command(name), std::forward<Args>(args)...)
    { }

     // Run the command
    void operator() () const;

     // Check if valid
    explicit operator bool () const { return !!command; }
};

namespace X {
    struct ConflictingCommandName : hacc::X::Error {
        String name;
        String desc_a;
        String desc_b;
        ConflictingCommandName(Str n, Str a, Str b) :
            name(n), desc_a(a), desc_b(b)
        { }
    };
    struct CommandNotFound : hacc::X::Error {
        String name;
        CommandNotFound(Str n) : name(n) { }
    };
    struct StatementWrongArgsType : hacc::X::Error {
        hacc::Type got;
        hacc::Type expected;
        StatementWrongArgsType(hacc::Type g, hacc::Type e) :
            got(g), expected(e)
        { }
    };
}

} // namespace control
