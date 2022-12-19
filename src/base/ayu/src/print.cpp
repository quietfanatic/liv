#include "../print.h"

#include <cstring>
#include <charconv>

#include "../compat.h"
#include "../describe-base.h"
#include "../exception.h"
#include "../type.h"
#include "char-cases-private.h"
#include "tree-private.h"

namespace ayu {
namespace in {

struct Printer {
    PrintOptions opts;
    char* start;
    char* end;

    Printer (PrintOptions f) :
        opts(f),
        start((char*)malloc(256)),
        end(start + 256)
    { }

    ~Printer () { free(start); }

    [[gnu::noinline]]
    [[nodiscard]]
    char* extend (char* p, usize more) {
        usize new_size = end - start;
        while (new_size < p - start + more) {
            new_size *= 2;
        }
        char* old_start = start;
        start = (char*)realloc(start, new_size);
        end = start + new_size;
        return p - old_start + start;
    }
    [[gnu::noinline]]
    [[nodiscard]]
    char* extend_1 (char* p) {
        usize new_size = (end - start) * 2;
        char* old_start = start;
        start = (char*)realloc(start, new_size);
        end = start + new_size;
        return p - old_start + start;
    }

    [[nodiscard]]
    char* pchar (char* p, char c) {
        if (p == end) [[unlikely]] p = extend_1(p);
        *p = c;
        return p+1;
    }
    [[nodiscard]]
    char* pstr (char* p, Str s) {
        if (p + s.size() > end) [[unlikely]] p = extend(p, s.size());
        std::memcpy(p, s.data(), s.size());
        return p + s.size();
    }

    [[nodiscard]]
    char* print_uint64 (char* p, uint64 v, bool hex) {
        if (v == 0) {
            return pchar(p, '0');
        }
        if (end - p < 20) [[unlikely]] p = extend(p, 20);
        auto [ptr, ec] = std::to_chars(
            p, p+20, v, hex ? 16 : 10
        );
        if (ec != std::errc()) AYU_INTERNAL_UGUU();
        return ptr;
    }

    [[nodiscard]]
    char* print_int64 (char* p, int64 v, bool hex) {
        if (v == 0) {
            return pchar(p, '0');
        }
        if (v < 0) {
            p = pchar(p, '-');
        }
        if (hex) {
            p = pstr(p, "0x"sv);
        }
        return print_uint64(p, v < 0 ? -v : v, hex);
    }

    [[nodiscard]]
    char* print_double (char* p, double v, bool hex) {
        if (hex) {
            if (v < 0) {
                p = pchar(p, '-');
                v = -v;
            }
            p = pstr(p, "0x"sv);
        }
        if (end - p < 24) [[unlikely]] p = extend(p, 24);
        auto [ptr, ec] = std::to_chars(
            p, p+24, v, hex
                ? std::chars_format::hex
                : std::chars_format::general
        );
        if (ec != std::errc()) AYU_INTERNAL_UGUU();
        return ptr;
    }

    [[nodiscard]]
    char* print_quoted (char* p, Str s, bool expand) {
        p = pchar(p, '"');
        for (auto c : s)
        switch (c) {
            case '"': p = pstr(p, "\\\""sv); break;
            case '\\': p = pstr(p, "\\\\"sv); break;
            case '\b': p = pstr(p, "\\b"sv); break;
            case '\f': p = pstr(p, "\\f"sv); break;
            case '\n':
                if (expand) p = pchar(p, c);
                else p = pstr(p, "\\n"sv);
                break;
            case '\r': p = pstr(p, "\\r"sv); break;
            case '\t':
                if (expand) p = pchar(p, c);
                else p = pstr(p, "\\t"sv);
                break;
            default: p = pchar(p, c); break;
        }
        return pchar(p, '"');
    }

    [[nodiscard]]
    char* print_string (char* p, Str s, bool expand) {
        if (opts & JSON) {
            return print_quoted(p, s, false);
        }
        if (s == ""sv || s == "null"sv || s == "true"sv || s == "false"sv) {
            return pchar(pstr(pchar(p, '"'), s), '"');
        }
        switch (s[0]) {
            case ANY_LETTER:
            case '_': break;
            default: return print_quoted(p, s, expand);
        }

        for (auto sp = s.begin(); sp != s.end(); sp++)
        switch (sp[0]) {
            case ':': {
                 // Don't need to check bounds because s should always be
                 // NUL-terminated
                if (sp[1] == ':' || sp[1] == '/') {
                    sp++;
                    continue;
                }
                else return print_quoted(p, s, expand);
            }
            case ANY_LETTER: case ANY_DECIMAL_DIGIT:
            case '-': case '.': case '/': case '_': continue;
            default: return print_quoted(p, s, expand);
        }
         // No need to quote
        return pstr(p, s);
    }

    [[nodiscard]]
    char* print_newline (char* p, uint n) {
        p = pchar(p, '\n');
        for (; n; n--) p = pstr(p, "    "sv);
        return p;
    }

    [[nodiscard]]
    char* print_subtree (char* p, const Tree& t, uint ind) {
        uint16 flags = t.data->flags;
        switch (t.data->rep) {
            case Rep::NULLREP: return pstr(p, "null"sv);
            case Rep::BOOL: {
                Str s = t.data->as_known<bool>() ? "true"sv : "false"sv;
                return pstr(p, s);
            }
            case Rep::INT64: {
                bool hex = !(opts & JSON) && flags & PREFER_HEX;
                return print_int64(p, t.data->as_known<int64>(), hex);
            }
            case Rep::DOUBLE: {
                double v = t.data->as_known<double>();
                if (v != v) {
                    return pstr(p, opts & JSON ? "null"sv : "+nan"sv);
                }
                else if (v == 1.0/0.0) {
                    return pstr(p, opts & JSON ? "1e999"sv : "+inf"sv);
                }
                else if (v == -1.0/0.0) {
                    return pstr(p, opts & JSON ? "-1e999"sv : "-inf"sv);
                }
                else if (v == 0) {
                    if (1.0/v == -inf) {
                        p = pchar(p, '-');
                    }
                    return pchar(p, '0');
                }
                else {
                    bool hex = !(opts & JSON) && flags & PREFER_HEX;
                    return print_double(p, v, hex);
                }
            }
            case Rep::STRING:
                return print_string(
                    p, t.data->as_known<String>(), flags & PREFER_EXPANDED
                );
            case Rep::ARRAY: {
                const Array& a = t.data->as_known<Array>();
                if (a.empty()) {
                    return pstr(p, "[]"sv);
                }

                 // Print "small" arrays compactly.
                 // TODO: Revise this?
                bool expand = !(opts & PRETTY) ? false
                            : flags & PREFER_EXPANDED ? true
                            : flags & PREFER_COMPACT ? false
                            : a.size() > 4;

                bool show_indices = expand
                                 && a.size() > 4
                                 && !(opts & JSON);
                p = pchar(p, '[');
                for (auto& elem : a) {
                    if (&elem == &a.front()) {
                        if (expand) p = print_newline(p, ind + 1);
                    }
                    else {
                        if (expand) {
                            if (opts & JSON) p = pchar(p, ',');
                            p = print_newline(p, ind + 1);
                        }
                        else {
                            p = pchar(p, opts & JSON ? ',' : ' ');
                        }
                    }
                    p = print_subtree(p, elem, ind + expand);
                    if (show_indices) {
                        p = pstr(p, "  // "sv);
                        p = print_int64(p, &elem - &a.front(), false);
                    }
                }
                if (expand) p = print_newline(p, ind);
                return pchar(p, ']');
            }
            case Rep::OBJECT: {
                const Object& o = t.data->as_known<Object>();
                if (o.empty()) {
                    return pstr(p, "{}"sv);
                }

                 // TODO: Decide what to do if both PREFER flags are set
                bool expand = !(opts & PRETTY) ? false
                            : flags & PREFER_EXPANDED ? true
                            : flags & PREFER_COMPACT ? false
                            : o.size() > 1;

                p = pchar(p, '{');
                for (auto& attr : o) {
                    if (&attr == &o.front()) {
                        if (expand) {
                            p = print_newline(p, ind + 1);
                        }
                    }
                    else {
                        if (expand) {
                            if (opts & JSON) p = pchar(p, ',');
                            p = print_newline(p, ind + 1);
                        }
                        else {
                            p = pchar(p, opts & JSON ? ',' : ' ');
                        }
                    }
                    p = print_string(p, attr.first, false);
                    p = pchar(p, ':');
                    if (expand) p = pchar(p, ' ');
                    p = print_subtree(p, attr.second, ind + expand);
                }
                if (expand) p = print_newline(p, ind);
                return pchar(p, '}');
            }
            case Rep::ERROR: {
                try {
                    std::rethrow_exception(
                        t.data->as_known<std::exception_ptr>()
                    );
                }
                catch (const std::exception& e) {
                    Str name;
                    try {
                        name = Type(typeid(e)).name();
                    }
                    catch (const UnknownType&) {
                        name = typeid(e).name();
                    }
                    return pchar(pstr(pstr(p, "?("sv), name), ')');
                }
                AYU_INTERNAL_UGUU();
            }
            default: AYU_INTERNAL_UGUU();
        }
    }
    [[nodiscard]]
    char* print_tree (char* p, const Tree& t) {
        p = print_subtree(p, t, 0);
        if (opts & PRETTY) p = pchar(p, '\n');
        return p;
    }
};

static void validate_print_options (PrintOptions opts) {
    if (opts & ~VALID_PRINT_OPTION_BITS) {
        throw X<InvalidPrintOptions>(opts);
    }
    if ((opts & PRETTY) && (opts & COMPACT)) {
        throw X<InvalidPrintOptions>(opts);
    }
}

} using namespace in;

String tree_to_string (const Tree& t, PrintOptions opts) {
    validate_print_options(opts);
    if (!(opts & PRETTY)) opts |= COMPACT;
    Printer printer (opts);
    char* p = printer.print_tree(printer.start, t);
    return String(printer.start, p - printer.start);
}

 // Forget C++ IO and its crummy diagnostics
void string_to_file (Str content, Str filename) {
    FILE* f = fopen_utf8(String(filename).c_str(), "wb");
    if (!f) {
        throw X<OpenFailed>(String(filename), errno);
    }
    fwrite(content.data(), 1, content.size(), f);
    if (fclose(f) != 0) {
        throw X<CloseFailed>(String(filename), errno);
    }
}

void tree_to_file (const Tree& t, Str filename, PrintOptions opts) {
    validate_print_options(opts);
    if (!(opts & COMPACT)) opts |= PRETTY;
    Printer printer (opts);
    char* p = printer.print_tree(printer.start, t);
    string_to_file(Str(printer.start, p - printer.start), filename);
}

} using namespace ayu;

AYU_DESCRIBE(ayu::InvalidPrintOptions,
    elems(
        elem(base<Error>(), inherit),
        elem(&InvalidPrintOptions::opts)
    )
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
    test(tree_to_string(Tree(1.0)), "1", "Autointification small");
    test(tree_to_string(Tree(145.0)), "145", "Autointification small");

    done_testing();
});
#endif
