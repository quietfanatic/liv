#include "command-builtins.h"

#include "../uni/utf.h"

namespace control::command {

static void echo_ (std::string s) {
    uni::print_utf8(s);
}
Command echo (echo_, "echo", "Print a string to stdout");

static void seq_ (const std::vector<Statement>& sts) {
    for (auto& st : sts) st();
}
Command seq (seq_, "seq", "Run multiple commands in a row");

} // namespace control::command
