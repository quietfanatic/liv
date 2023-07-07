#include "command-builtins.h"

#include "../uni/utf.h"

namespace control::command {

static void echo_ (AnyString s) {
    uni::print_utf8(s);
}
Command echo (echo_, "echo", "Print a string to stdout");

static void seq_ (const UniqueArray<Statement>& sts) {
    for (auto& st : sts) st();
}
Command seq (seq_, "seq", "Run multiple commands in a row");

static void help_ (std::optional<AnyString>) {
    uni::print_utf8("help is NYI, sorry");
}
Command help (help_, "help", "NYI");

} // namespace control::command
