#include "command.h"

#include "../ayu/describe.h"

namespace control {

static std::unordered_map<Str, const Command*>& commands_by_name () {
    static std::unordered_map<Str, const Command*> r;
    return r;
}

void Command::register_command () const {
    auto [iter, emplaced] = commands_by_name().emplace(name, this);
    if (!emplaced) {
        throw ayu::X<ConflictingCommandName>(
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
    else throw ayu::X<CommandNotFound>(std::string(name));
}

Statement::Statement (Command* c, ayu::Dynamic&& a) : command(c), args(std::move(a)) {
    if (args.type != command->args_type()) {
        throw ayu::X<StatementWrongArgsType>(args.type, command->args_type());
    }
}

 // Should this be inlined?
void Statement::operator() () const {
#ifndef DEBUG
    require(args.type == command->args_type());
#endif
    command->wrapper(command->function, args.data);
}

} using namespace control;

AYU_DESCRIBE(const Command*,
    delegate(const_ref_funcs<std::string>(
        [](const Command* const& c)->const std::string&{
            return c->name;
        },
        [](const Command*& c, const std::string& s){
            c = require_command(s);
        }
    ))
)

AYU_DESCRIBE(Statement,
    to_tree([](const Statement& s){
         // Serialize the args and stick the command name in front
         // TODO: allow constructing readonly Reference from const Dynamic
        auto args_tree = ayu::item_to_tree(const_cast<ayu::Dynamic&>(s.args).ptr());
        auto a = ayu::Array(args_tree);
        a.emplace(a.begin(), s.command->name);
        return ayu::Tree(a);
    }),
    from_tree([](Statement& s, const ayu::Tree& t){
         // Get the command from the first elem, then args from the rest.
         // TODO: optional parameters
        auto& a = static_cast<const ayu::Array&>(t);
        if (a.size() == 0) {
            s = {}; return;
        }
        s.command = require_command(Str(a[0]));
        ayu::Array args_a;
        for (usize i = 1; i < a.size(); i++) {
            args_a.push_back(a[i]);
        }
        s.args = ayu::Dynamic(s.command->args_type());
        ayu::item_from_tree(
            s.args.ptr(), ayu::Tree(args_a), ayu::Location(), ayu::DELAY_SWIZZLE
        );
    })
)

AYU_DESCRIBE(control::ConflictingCommandName,
    delegate(base<ayu::Error>()),
    elems(
        elem(&control::ConflictingCommandName::name),
        elem(&control::ConflictingCommandName::desc_a),
        elem(&control::ConflictingCommandName::desc_b)
    )
)

AYU_DESCRIBE(control::CommandNotFound,
    delegate(base<ayu::Error>()),
    elems(
        elem(&control::CommandNotFound::name)
    )
)

AYU_DESCRIBE(control::StatementWrongArgsType,
    delegate(base<ayu::Error>()),
    attrs(
        attr("expected", &control::StatementWrongArgsType::expected),
        attr("got", &control::StatementWrongArgsType::got)
    )
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
        ayu::item_from_string(&s, "[_test_command 5 6]");
    }, "Can create command from ayu");
    doesnt_throw([&]{
        s();
    }, "Can call command");
    is(test_vals.back(), 30, "Command gave correct result");

    is(ayu::item_to_string(&s), "[_test_command 5 6]",
        "Command serializes correctly"
    );

    throws<ayu::Error>([&]{
        ayu::item_from_string(&s, "[_test_command]");
    }, "Can't create command with too few args");

    throws<ayu::Error>([&]{
        ayu::item_from_string(&s, "[_test_command 1 2 3]");
    }, "Can't create command with too many args");

    test_vals = {};
    doesnt_throw([&]{
        ayu::item_from_string(&s, "[seq [[_test_command 5 6] [_test_command 7 8]]]");
        s();
    }, "seq command");
    is(test_vals.size(), usize(2), "seq command works");

    done_testing();
});
#endif
