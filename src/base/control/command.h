// A function type that can be used with ayu, to make a non-turing-complete
// imperative DSL.

#pragma once

#include <utility>
#include "../ayu/dynamic.h" 
 // For tuple describability
#include "../ayu/describe-standard.h"
#include "../ayu/type.h"
#include "../uni/common.h"
#include "../uni/macros.h"
#include "command-template-utils.h"

namespace control {
using namespace uni;

template <class F>
using Function = F;

 // This is how you define new commands.  Make static objects of these.
struct Command {
    Function<void(void*, void*)>* wrapper;
    void* function;
    String name;
    String description;
    usize required_arg_count;
    Function<ayu::Type()>* args_type;
    Function<std::vector<ayu::Type>()>* arg_types;

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
            return ayu::Type::CppType<StatementStorage<Args...>>();
        }),
        arg_types([]{
            static std::vector<ayu::Type> r = {ayu::Type::CppType<Args>()...};
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
 // but it doesn't support optional arguments unless you deserialize from ayu.
struct Statement {
    const Command* command = null;
    ayu::Dynamic args;  // Type must be command->args_type (std::tuple)

    CE Statement() { }
    Statement (Command* c, ayu::Dynamic&& a);
    template <class... Args>
    Statement (Command* c, Args... args) :
        Statement(c, ayu::Dynamic(StatementStorage<Args...>(std::forward<Args>(args)...)))
    { }
    template <class... Args>
    Statement (Str name, Args... args) :
        Statement(require_command(name), std::forward<Args>(args)...)
    { }

     // Run the command
    void operator() () const;

    explicit operator bool () const { return !!command; }
};

namespace X {
    struct ConflictingCommandName : ayu::X::Error {
        String name;
        String desc_a;
        String desc_b;
        ConflictingCommandName(Str n, Str a, Str b) :
            name(n), desc_a(a), desc_b(b)
        { }
    };
    struct CommandNotFound : ayu::X::Error {
        String name;
        CommandNotFound(Str n) : name(n) { }
    };
    struct StatementWrongArgsType : ayu::X::Error {
        ayu::Type expected;
        ayu::Type got;
        StatementWrongArgsType(ayu::Type e, ayu::Type g) :
            expected(e), got(g)
        { }
    };
}

} // namespace control
