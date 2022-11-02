#include "../print.h"

#include <charconv>

#include "../compat.h"
#include "../type.h"
#include "char-cases-private.h"
#include "tree-private.h"

using namespace std::string_literals;

namespace ayu {
using namespace in;

 // TODO: Keep newlines in non-compact layout?
static String print_quoted (Str s) {
    String r = "\""s;
    for (auto p = s.begin(); p != s.end(); p++)
    switch (*p) {
        case '"': r += "\\\""; break;
        case '\\': r += "\\\\"; break;
        case '\b': r += "\\b"; break;
        case '\f': r += "\\f"; break;
        case '\n': r += "\\n"; break;
        case '\r': r += "\\r"; break;
        case '\t': r += "\\t"; break;
        default: r += String(1, *p); break;
    }
    return r + '"';
}

static String print_string (Str s) {
    if (s == "" || s == "null" || s == "true" || s == "false") {
        return print_quoted(s);
    }
    switch (s[0]) {
        case ANY_LETTER:
        case '_': break;
        default: return print_quoted(s);
    }

    for (auto p = s.begin(); p != s.end(); p++)
    switch (p[0]) {
        case ':': {
            if (p[1] == ':') {
                p++;
                continue;
            }
            else return print_quoted(s);
        }
        case ANY_LETTER: case ANY_DECIMAL_DIGIT:
        case '-': case '.': case '/': case '_': continue;
        default: return print_quoted(s);
    }
    return String(s);
}

static String indent (uint n) {
    String r = "";
    for (; n; n--) r += "    ";
    return r;
}

String print_tree (const Tree& t, PrintFlags flags, uint ind) {
    switch (t.data->rep) {
        case Rep::NULLREP: return "null";
        case Rep::BOOL: return t.data->as_known<bool>() ? "true" : "false";
        case Rep::INT64: return std::to_string(t.data->as_known<int64>());
        case Rep::DOUBLE: {
            double v = t.data->as_known<double>();
            if (v != v) return "+nan";
            else if (v == 1.0/0.0) return "+inf";
            else if (v == -1.0/0.0) return "-inf";
            else {
                char buf [32]; // Should be enough?
                auto [ptr, ec] = std::to_chars(buf, buf+32, v);
                if (ptr == buf) AYU_INTERNAL_UGUU();
                return String(buf, ptr - buf);
            }
        }
        case Rep::STRING: return print_string(t.data->as_known<String>());
        case Rep::ARRAY: {
            const Array& a = t.data->as_known<Array>();
            if (a.size() == 0) return "[]"s;

             // Print "small" arrays compactly.
            bool print_compact = (flags & COMPACT) || a.size() == 1 || [&]{
                usize n_elems = 0;
                bool contains_array = false;
                for (auto& e : a) {
                    if (e.form() == ARRAY) {
                        n_elems += e.data->as_known<Array>().size();
                        contains_array = true;
                    }
                    else if (e.form() == OBJECT) {
                        return false;
                    }
                    else n_elems += 1;
                }
                return !contains_array || n_elems <= 8;
            }();

            bool show_indices = !print_compact && a.size() > 3;
            String r = "[";
            for (auto& e : a) {
                if (print_compact) {
                    if (&e != &a.front()) r += " ";
                }
                else {
                    r += "\n" + indent(ind + 1);
                }
                r += print_tree(e, flags, ind + !print_compact);
                if (show_indices) {
                    r += "  # " + std::to_string(&e - &a.front());
                }
            }
            if (!print_compact)
                r += "\n" + indent(ind);
            return r + "]";
        }
        case Rep::OBJECT: {
            const Object& o = t.data->as_known<Object>();
            if (o.size() == 0) return "{}";
            bool print_compact = (flags & COMPACT) || o.size() == 1;
            String r = "{";
            auto nexti = o.begin();
            for (auto i = nexti; i != o.end(); i = nexti) {
                if (print_compact) {
                    if (nexti != o.begin()) r += " ";
                }
                else {
                    r += "\n" + indent(ind + 1);
                }
                r += print_string(i->first);
                r += ":";
                r += print_tree(i->second, flags, ind + !print_compact);
                nexti++;
            }
            if (!print_compact)
                r += "\n" + indent(ind);
            return r + "}";
        }
        case Rep::ERROR: {
            try {
                std::rethrow_exception(t.data->as_known<std::exception_ptr>());
            }
            catch (const X::Error& e) {
                try {
                    return "?(" + Type(typeid(e)).name() + ")";
                }
                catch (const X::UnknownType&) {
                    return "?("s + typeid(e).name() + ")";
                }
            }
        }
        default: AYU_INTERNAL_UGUU();
    }
}

String tree_to_string (const Tree& t, PrintFlags flags) {
    String r = print_tree(t, flags, 0);
    if (!(flags & COMPACT)) r += '\n';
    return r;
}

 // Forget C++ IO and its crummy diagnostics
void string_to_file (Str content, Str filename) {
    FILE* f = fopen_utf8(String(filename).c_str(), "wb");
    if (!f) {
        throw X::OpenFailed(filename, errno);
    }
    fwrite(content.data(), 1, content.size(), f);
    if (fclose(f) != 0) {
        throw X::CloseFailed(filename, errno);
    }
}

void tree_to_file (const Tree& tree, Str filename, PrintFlags flags) {
    return string_to_file(tree_to_string(tree, flags), filename);
}

} using namespace ayu;

#ifndef TAP_DISABLE_TESTS
#include "../../tap/tap.h"

static tap::TestSet tests ("base/ayu/print", []{
    using namespace tap;
    auto t = [](const Tree& t, const char* s){
        is(tree_to_string(t, ayu::COMPACT), s, s);
    };
    t(Tree(null), "null");
    t(Tree(345), "345");
    t(Tree(-44), "-44");
    t(Tree(2.5), "2.5");
    t(Tree(-4.0), "-4");
    t(Tree(0.0/0.0), "+nan");
    t(Tree(1.0/0.0), "+inf");
    t(Tree(-1.0/0.0), "-inf");
    t(Tree(""), "\"\"");
    t(Tree("asdf"), "asdf");
    t(Tree("null"), "\"null\"");
    t(Tree("true"), "\"true\"");
    t(Tree("false"), "\"false\"");
    t(Tree(Array{}), "[]");
    t(Tree(Array{Tree(0), Tree(1), Tree("foo")}), "[0 1 foo]");
    t(Tree(Object{}), "{}");
    t(Tree(Object{
        Pair{"a", Tree(0)},
        Pair{"null", Tree(1)},
        Pair{"0", Tree("foo")}
    }), "{a:0 \"null\":1 \"0\":foo}");
    t(Tree(Array{
        Tree(Array{Tree(0), Tree(1)}),
        Tree(Array{
            Tree(Array{Tree(2)}),
            Tree(Array{Tree(3), Tree(4)})
        })
    }), "[[0 1] [[2] [3 4]]]");
    done_testing();
});
#endif
