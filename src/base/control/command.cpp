#include "command.h"

#include "../hacc/haccable.h"

namespace control {

static std::unordered_map<Str, const Command*>& commands_by_name () {
    static std::unordered_map<Str, const Command*> r;
    return r;
}

void Command::register_command () const {
    auto [iter, emplaced] = commands_by_name().emplace(name, this);
    if (!emplaced) {
        throw X::ConflictingCommandName(
            name, iter->second->description, description
        );
    }
}

const Command* lookup_command (Str name) {
    auto iter = commands_by_name().find(name);
    if (iter != commands_by_name().end()) return iter->second;
    else return nullptr;
}
const Command* require_command (Str name) {
    auto iter = commands_by_name().find(name);
    if (iter != commands_by_name().end()) return iter->second;
    else throw X::CommandNotFound(name);
}

Statement::Statement (Command* c, hacc::Dynamic&& a) : command(c), args(std::move(a)) {
    if (args.type != command->args_type()) {
        throw X::StatementWrongArgsType(args.type, command->args_type());
    }
}

 // Should this be inlined?
void Statement::operator() () const {
#ifndef DEBUG
    AA(args.type == command->args_type());
#endif
    command->wrapper(command->function, args.data);
}

} using namespace control;

HACCABLE(const Command*,
    delegate(const_ref_funcs<String>(
        [](const Command* const& c)->const String&{
            return c->name;
        },
        [](const Command*& c, const String& s){
            c = require_command(s);
        }
    ))
)

HACCABLE(Statement,
    to_tree([](const Statement& s){
         // Serialize the args and stick the command name in front
         // TODO: allow constructing readonly Reference from const Dynamic
        auto args_tree = hacc::item_to_tree(const_cast<hacc::Dynamic&>(s.args));
        auto a = hacc::Array(args_tree);
        a.emplace(a.begin(), s.command->name);
        return hacc::Tree(a);
    }),
    from_tree([](Statement& s, const hacc::Tree& t){
         // Get the command from the first elem, then args from the rest.
         // TODO: optional parameters
        auto a = hacc::Array(t);
        if (a.size() == 0) {
            s = {}; return;
        }
        s.command = require_command(Str(a[0]));
        hacc::Array args_a;
        for (usize i = 1; i < a.size(); i++) {
            args_a.push_back(a[i]);
        }
        s.args = hacc::Dynamic(s.command->args_type());
        hacc::item_from_tree(s.args, hacc::Tree(args_a));
    })
)

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"

static std::vector<int> test_vals;
static void test_command_ (int a, int b) {
    test_vals.push_back(a * b);
}
static Command test_command (test_command_, "_test_command", "Command for testing, do not use.", 1);

static tap::TestSet tests ("base/control/command", []{
    using namespace tap;

    Statement s (&test_command, 3, 4);
    doesnt_throw([&]{
        s();
    }, "Can create a command in C++");
    is(test_vals.size(), usize(1), "Can call command");
    is(test_vals.back(), 12, "Command gave correct result");

    s = Statement();

    doesnt_throw([&]{
        hacc::item_from_string(&s, "[_test_command 5 6]");
    }, "Can create command from hacc");
    doesnt_throw([&]{
        s();
    }, "Can call command");
    is(test_vals.back(), 30, "Command gave correct result");

    is(hacc::item_to_string(&s, hacc::COMPACT), "[_test_command 5 6]",
        "Command serializes correctly"
    );

    throws<hacc::X::Error>([&]{
        hacc::item_from_string(&s, "[_test_command]");
    }, "Can't create command with too few args");

    throws<hacc::X::Error>([&]{
        hacc::item_from_string(&s, "[_test_command 1 2 3]");
    }, "Can't create command with too many args");

    test_vals = {};
    doesnt_throw([&]{
        hacc::item_from_string(&s, "[seq [[_test_command 5 6] [_test_command 7 8]]]");
        s();
    }, "seq command");
    is(test_vals.size(), usize(2), "seq command works");

    done_testing();
});
#endif
