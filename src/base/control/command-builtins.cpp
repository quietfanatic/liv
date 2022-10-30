#include "command-builtins.h"

#include "../hacc/compat.h"

namespace control::command {

static void echo_ (String s) {
    hacc::print_utf8(s);
}
Command echo (echo_, "echo", "Print a string to stdout");

} // namespace control::command
