#include "../print.h"

#include <charconv>

#include "../compat.h"
#include "../describe-base.h"
#include "../type.h"
#include "char-cases-private.h"
#include "tree-private.h"

namespace ayu {
namespace in {

struct Printer {
    String& out;
    PrintOptions opts;

    Printer (String& o, PrintOptions f) : out(o), opts(f) { }

     // TODO: Keep newlines in non-compact layout?
    void print_quoted (Str s) {
        out += '"';
        for (auto p = s.begin(); p != s.end(); p++)
        switch (*p) {
            case '"': out += "\\\""sv; break;
            case '\\': out += "\\\\"sv; break;
            case '\b': out += "\\b"sv; break;
            case '\f': out += "\\f"sv; break;
            case '\n': out += "\\n"sv; break;
            case '\r': out += "\\r"sv; break;
            case '\t': out += "\\t"sv; break;
            default: out += String(1, *p); break;
        }
        out += '"';
    }

    void print_string (Str s) {
        if (opts & JSON) return print_quoted(s);
        if (s == ""sv) {
            out += "\"\""sv; return;
        }
        else if (s == "null"sv) {
            out += "\"null\""sv; return;
        }
        else if (s == "true"sv) {
            out += "\"true\""sv; return;
        }
        else if (s == "false"sv) {
            out += "\"false\""sv; return;
        }
        switch (s[0]) {
            case ANY_LETTER:
            case '_': break;
            default: return print_quoted(s);
        }

        for (auto p = s.begin(); p != s.end(); p++)
        switch (p[0]) {
            case ':': {
                 // Don't need to check bounds because String is NUL-terminated
                if (p[1] == ':' || p[1] == '/') {
                    p++;
                    continue;
                }
                else return print_quoted(s);
            }
            case ANY_LETTER: case ANY_DECIMAL_DIGIT:
            case '-': case '.': case '/': case '_': continue;
            default: return print_quoted(s);
        }
         // No need to quote
        out += s;
    }

    void print_newline (uint n) {
        out += '\n';
        for (; n; n--) out += "    "sv;
    }

    void print_tree (const Tree& t, uint ind) {
        switch (t.data->rep) {
            case Rep::NULLREP:
                out += "null"sv;
                return;
            case Rep::BOOL:
                out += (t.data->as_known<bool>()
                    ? "true"sv : "false"sv
                );
                return;
            case Rep::INT64:
                out += std::to_string(t.data->as_known<int64>());
                return;
            case Rep::DOUBLE: {
                double v = t.data->as_known<double>();
                if (v != v) {
                    if (opts & JSON) out += "null";
                    else out += "+nan"sv;
                }
                else if (v == 1.0/0.0) {
                    if (opts & JSON) out += "1e999";
                    else out += "+inf"sv;
                }
                else if (v == -1.0/0.0) {
                    if (opts & JSON) out += "-1e999";
                    else out += "-inf"sv;
                }
                else {
                    char buf [32]; // Should be enough?
                    auto [ptr, ec] = std::to_chars(buf, buf+32, v);
                    if (ptr == buf) AYU_INTERNAL_UGUU();
                    out += Str(buf, ptr - buf);
                }
                return;
            }
            case Rep::STRING:
                return print_string(t.data->as_known<String>());
            case Rep::ARRAY: {
                const Array& a = t.data->as_known<Array>();
                if (a.empty()) { out += "[]"sv; return; }

                 // Print "small" arrays compactly.
                 // TODO: Revise this?
                bool expand = (opts & PRETTY) && a.size() > 4;

                bool show_indices = expand
                                 && a.size() > 4
                                 && !(opts & JSON);
                out += '[';
                for (auto& elem : a) {
                    if (&elem == &a.front()) {
                        if (expand) print_newline(ind + 1);
                    }
                    else {
                        if (expand) {
                            if (opts & JSON) out += ',';
                            print_newline(ind + 1);
                        }
                        else {
                            if (opts & JSON) out += ',';
                            else out += ' ';
                        }
                    }
                    print_tree(elem, ind + expand);
                    if (show_indices) {
                        out = cat(out, "  // "sv, (&elem - &a.front()));
                    }
                }
                if (expand) print_newline(ind);
                out += ']';
                return;
            }
            case Rep::OBJECT: {
                const Object& o = t.data->as_known<Object>();
                if (o.empty()) { out += "{}"sv; return; }

                bool expand = (opts & PRETTY) && o.size() > 1;

                out += '{';
                for (auto& attr : o) {
                    if (&attr == &o.front()) {
                        if (expand) print_newline(ind + 1);
                    }
                    else {
                        if (expand) {
                            if (opts & JSON) out += ',';
                            print_newline(ind + 1);
                        }
                        else {
                            if (opts & JSON) out += ',';
                            else out += ' ';
                        }
                    }
                    print_string(attr.first);
                    out += ':';
                    if (expand) out += ' ';
                    print_tree(attr.second, ind + expand);
                }
                if (expand) print_newline(ind);
                out += '}';
                return;
            }
            case Rep::ERROR: {
                try {
                    std::rethrow_exception(
                        t.data->as_known<std::exception_ptr>()
                    );
                }
                catch (const X::Error& e) {
                    try {
                        out += cat("?("sv, Type(typeid(e)).name(), ')');
                    }
                    catch (const X::UnknownType&) {
                        out += cat("?("sv, typeid(e).name(), ')');
                    }
                }
            }
            default: AYU_INTERNAL_UGUU();
        }
    }
};

static void validate_print_options (PrintOptions opts) {
    if (opts & ~VALID_PRINT_OPTION_BITS) {
        throw X::InvalidPrintOptions(opts);
    }
    if ((opts & PRETTY) && (opts & COMPACT)) {
        throw X::InvalidPrintOptions(opts);
    }
}

} using namespace in;

String tree_to_string (const Tree& t, PrintOptions opts) {
    validate_print_options(opts);
    if (!(opts & PRETTY)) opts |= COMPACT;
    String r;
    Printer(r, opts).print_tree(t, 0);
    if (opts & PRETTY) r += '\n';
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

void tree_to_file (const Tree& tree, Str filename, PrintOptions opts) {
    validate_print_options(opts);
    if (!(opts & COMPACT)) opts |= PRETTY;
    return string_to_file(tree_to_string(tree, opts), filename);
}

} using namespace ayu;

AYU_DESCRIBE(ayu::X::InvalidPrintOptions,
    delegate(base<X::Error>()),
    elems(elem(&X::InvalidPrintOptions::opts))
)

#ifndef TAP_DISABLE_TESTS
#include "../../tap/tap.h"
#include "../parse.h"
#include "../resource.h"
#include "test-environment-private.h"

static tap::TestSet tests ("base/ayu/print", []{
    using namespace tap;

    test::TestEnvironment env;

    String pretty = string_from_file(resource_filename("ayu-test:/print-pretty.ayu"));
    String compact = string_from_file(resource_filename("ayu-test:/print-compact.ayu"));
    String pretty_json = string_from_file(resource_filename("ayu-test:/print-pretty.json"));
    String compact_json = string_from_file(resource_filename("ayu-test:/print-compact.json"));
     // Remove final LF
    compact.pop_back();
    compact_json.pop_back();

    Tree t = tree_from_string(pretty);

    auto test = [](Str got, Str expected, String name){
        if (!is(got, expected, name)) {
            usize i = 0;
            for (; i < got.size() && i < expected.size(); i++) {
                if (got[i] != expected[i]) {
                    diag(cat("First difference at ",
                        i, " |", got[i], '|', expected[i], '|'
                    ));
                    return;
                }
            }
            if (got.size() != expected.size()) {
                diag(cat("Size difference got ",
                    got.size(), " expected ", expected.size()
                ));
            }
        }
    };
    test(tree_to_string(t, PRETTY), pretty, "Pretty");
    test(tree_to_string(t, COMPACT), compact, "Compact");
    test(tree_to_string(t, PRETTY|JSON), pretty_json, "Pretty JSON");
    test(tree_to_string(t, COMPACT|JSON), compact_json, "Compact JSON");

    done_testing();
});
#endif
