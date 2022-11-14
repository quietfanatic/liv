#include "../parse.h"

#include <cstring>
#include <charconv>
#include <limits>

#include "../compat.h"
#include "../describe.h"
#include "../print.h"
#include "char-cases-private.h"

namespace ayu {

namespace in {

 // Parsing is simple enough that we don't need a separate lexer step.
struct Parser {
    String filename;
    const char* begin;
    const char* p;
    const char* end;

     // std::unordered_map is supposedly slow, so we'll use std::vector instead.
     // We'll rethink if we ever need to parse a document with a large amount
     // of shortcuts (I can't imagine for my use cases having more than 20
     // or so).
    Object shortcuts;

    Parser (Str s, Str filename = ""sv) :
        filename(filename),
        begin(s.data()),
        p(s.data()),
        end(s.data()+s.size())
    { }

     // Utility
    int look (int i = 0) { return p+i >= end ? EOF : p[i]; }

     // Error reporting
    char show_hex_digit (int d) {
        if (d > 9) return 'A' + d;
        else return '0' + d;
    }
    String show_char (int c) {
        switch (c) {
            case EOF: return "<EOF>"s;
            case ' ': return "<space>"s;
            default: {
                if (c > 0x20 && c < 0x7e) {
                    return String(1, c);
                }
                else {
                    return cat(
                        '<', show_hex_digit((c >> 4) & 0x0f),
                        show_hex_digit(c & 0xf), '>'
                    );
                }
            }
        }
    }
    template <class... Args>
    X::ParseError error (Args&&... args) {
         // Diagnose line and column number
         // I'm not sure the col is exactly right
        uint line = 1;
        const char* nl = begin - 1;
        for (const char* p2 = begin; p2 != p; p2++) {
            if (*p2 == '\n') {
                line++;
                nl = p2;
            }
        }
        uint col = p - nl;
        return X::ParseError(cat(std::forward<Args>(args)...), filename, line, col);
    }

    void skip_comment () {
        p += 2;  // for two /s
        for (;;) switch (look()) {
            case EOF: return;
            case '\n': p++; return;
            default: p++; break;
        }
    }
    void skip_ws () {
        for (;;) switch (look()) {
            case ANY_WS: p++; break;
            case '/': {
                if (end - p > 1 && p[1] == '/') {
                    skip_comment();
                }
                break;
            }
            default: return;
        }
    }
    void skip_commas () {
        for (;;) switch (look()) {
            case ANY_WS:
            case ',': p++; break;
            case '/': {
                if (end - p > 1 && p[1] == '/') {
                    skip_comment();
                    break;
                }
                else return;
            }
            default: return;
        }
    }

    String got_string () {
        p++;  // for the "
        String r;
        for (;;) switch (look()) {
            case EOF: throw error("String not terminated by end of input"sv);
            case '"': p++; return r;
            case '\\': {
                p++;
                switch (look()) {
                    case EOF: throw error("String not terminated by end of input"sv);
                    case '"': r += '"'; break;
                    case '\\': r += '\\'; break;
                    case '/': r += '/'; break;  // Dunno why this is in json
                    case 'b': r += '\b'; break;
                    case 'f': r += '\f'; break;
                    case 'n': r += '\n'; break;
                    case 'r': r += '\r'; break;
                    case 't': r += '\t'; break;
                    default: throw error("Unrecognized escape sequence \\"sv, show_char(look()));
                }
                p++;
                break;
            }
            default: r += *p++;
        }
    }

    Str got_word () {
        const char* start = p;
        p++;  // For the first character
        for (;;) switch (look()) {
            case ANY_LETTER: case ANY_DECIMAL_DIGIT: case ANY_WORD_SYMBOL:
                p++; break;
            case ':': {
                 // Allow :: (for c++ types) or :/ (for urls)
                if (look(1) == ':' || look(1) == '/') {
                    p += 2;
                    break;
                }
                else return Str(start, p - start);
            }
            case '"': {
                throw error("\" cannot occur inside a word (are you missing the first \"?)"sv);
            }
            case ANY_RESERVED_SYMBOL: {
                throw error(*p, " is a reserved symbol and can't be used outside of strings."sv);
            }
            default: return Str(start, p - start);
        }
    }

    Tree got_number () {
         // Detect special numbers
        if (end - p >= 4) {
            Str p4 (p, 4);
            double special_number = 0;
            if (p4 == "+nan"sv) {
                special_number = std::numeric_limits<double>::quiet_NaN();
            }
            else if (p4 == "+inf"sv) {
                special_number = std::numeric_limits<double>::infinity();
            }
            else if (p4 == "-inf"sv) {
                special_number = -std::numeric_limits<double>::infinity();
            }
            else goto no_special_number;
             // Make sure there's no junk at the end
            p += 4;
            if (end != p) switch (look()) {
                case ANY_LETTER: case ANY_DECIMAL_DIGIT: case ANY_WORD_SYMBOL:
                    throw error("Malformed_number"sv);
                default: break;
            }
            return Tree(special_number);
            no_special_number:;
        }
         // Detect sign
        bool minus = false;
        switch (look()) {
            case '+': {
                p++;
                if (look() == '-') throw error("Malformed number"sv);
                break;
            }
            case '-': {
                minus = true;
                p++;
                if (look() == '-') throw error("Malformed number"sv);
                break;
            }
            default: break;
        }
         // Detect hex prefix
        bool hex = false;
        if (end - p >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
            p += 2;
            hex = true;
        }
         // Try integer
        {
            int64 integer;
            auto [ptr, ec] = std::from_chars(p, end, integer, hex ? 16 : 10);
            if (ptr == p) {
                 // If the integer parse failed, the float parse will also fail.
                throw error("Malformed number"sv);
            }
             // Is this really an integer?
            if (ptr != end) switch (*ptr) {
                case '.': case 'e': case 'E': case 'p': case 'P': {
                    goto try_floating;
                }
                case ANY_INVALID_NUMBER_ENDER: {
                    throw error("Junk at end of number"sv);
                }
                default: break;
            }
            p = ptr;
            return Tree(minus ? -integer : integer);
        }
        try_floating: {
            double floating;
            auto [ptr, ec] = std::from_chars(
                p, end, floating,
                hex ? std::chars_format::hex
                    : std::chars_format::general
            );
            if (ptr == p) {
                throw error("Malformed number"sv);
            }
            if (ptr[-1] == '.') {
                throw error("Number cannot end with ."sv);
            }
            if (ptr != end) switch (*ptr) {
                case ANY_LETTER:
                case ANY_WORD_SYMBOL: {
                    throw error("Junk at end of number"sv);
                }
                default: break;
            }
            p = ptr;
            return Tree(minus ? -floating : floating);
        }
    }

    Array got_array () {
        Array a;
        p++;  // for the [
        for (;;) {
            skip_commas();
            switch (look()) {
                case EOF: throw error("Array not terminated"sv);
                case ':': throw error("Cannot have : in an array"sv);
                case ']': p++; return a;
                default: a.push_back(parse_term()); break;
            }
        }
    }

    Object got_object () {
        Object o;
        p++;  // for the {
        for (;;) {
            skip_commas();
            switch (look()) {
                case EOF: throw error("Object not terminated"sv);
                case ':': throw error("Missing key before : in object"sv);
                case '}': p++; return o;
                default: break;
            }
            Tree key = parse_term();
            if (key.form() != STRING) {
                throw error("Can't use non-string "sv, tree_to_string(key), " as key in object"sv);
            }
            skip_ws();
            switch (look()) {
                case EOF: throw error("Object not terminated"sv);
                case ':': p++; break;
                case ANY_RESERVED_SYMBOL: {
                    throw error(*p, " is a reserved symbol and can't be used outside of strings."sv);
                }
                default: throw error("Missing : after name in object"sv);
            }
            skip_ws();
            switch (look()) {
                case ',':
                case '}': throw error("Missing value after : in object"sv);
                default: {
                    o.emplace_back(String(key), parse_term());
                    break;
                }
            }
        }
        return o;
    }

    void set_shortcut (Str name, Tree value) {
        for (auto& p : shortcuts) {
            if (p.first == name) {
                throw error(
                    "Duplicate declaration of shortcut &"sv,
                    tree_to_string(Tree(name))
                );
            }
        }
        shortcuts.emplace_back(String(name), std::move(value));
    }
    const Tree& get_shortcut (Str name) {
        for (auto& p : shortcuts) {
            if (p.first == name) return p.second;
        }
        throw error(
            "Unknown shortcut *"sv,
            tree_to_string(Tree(name))
        );
    }

    Tree got_decl () {
        p++;  // for the &
        switch (look()) {
            case ANY_LETTER:
            case '_':
            case '"': break;
            default: throw error("Expected ref name after &"sv);
        }
        Tree name = parse_term();
        if (name.form() != STRING) {
            throw error("Can't use non-string "sv, tree_to_string(name), " as ref name"sv);
        }
        skip_ws();
        switch (look()) {
            case ':': {
                p++;
                skip_ws();
                set_shortcut(Str(name), parse_term());
                skip_commas();
                return parse_term();
            }
            default: {
                Tree value = parse_term();
                set_shortcut(Str(name), value);
                return value;
            }
        }
    }

    Tree got_shortcut () {
        p++;  // for the *
        switch (look()) {
            case ANY_LETTER:
            case '_':
            case '"': break;
            default: throw error("Expected ref name after *"sv);
        }
        Tree name = parse_term();
        if (name.form() != STRING) {
            throw error("Can't use non-string "sv, tree_to_string(name), " as ref name"sv);
        }
        return get_shortcut(Str(name));
    }

    Tree parse_term () {
        switch (look()) {
            case EOF: throw error("Expected term but ran into end of document"sv);
            case ANY_WORD_STARTER: {
                Str word = got_word();
                if (word == "null"sv) return Tree(null);
                else if (word == "true"sv) return Tree(true);
                else if (word == "false"sv) return Tree(false);
                else return Tree(word);
            }

            case ANY_DECIMAL_DIGIT:
            case '+':
            case '-': return got_number();

            case '"': return Tree(got_string());
            case '[': return Tree(got_array());
            case '{': return Tree(got_object());

            case '&': return got_decl();
            case '*': return got_shortcut();

            case ':':
            case ',':
            case ']':
            case '}': throw error("Unexpected "sv + *p);
            case ANY_RESERVED_SYMBOL:
                throw error(*p, " is a reserved symbol and can't be used outside of strings."sv);
            default: throw error("Unrecognized character "sv, *p);
        }
    }

    Tree parse () {
         // Skip BOM
        if (p + 2 < end && p[0] == char(0xef)
                        && p[1] == char(0xbb)
                        && p[2] == char(0xbf)
        ) p += 3;
        skip_ws();
        Tree r = parse_term();
        skip_ws();
        if (p != end) throw error("Extra stuff at end of document"sv);
        return r;
    }
};

} using namespace in;

 // Finally:
Tree tree_from_string (Str s, Str filename) {
    return Parser(s, filename).parse();
}

String string_from_file (Str filename) {
    FILE* f = fopen_utf8(String(filename).c_str(), "rb");
    if (!f) {
        throw X::OpenFailed(filename, errno);
    }

    fseek(f, 0, SEEK_END);
    usize size = ftell(f);
    rewind(f);

    String r (size, 0);
    usize did_read = fread(const_cast<char*>(r.data()), 1, size, f);
    if (did_read != size) {
        throw X::ReadFailed(filename, errno);
    }

    if (fclose(f) != 0) {
        throw X::CloseFailed(filename, errno);
    }
    return r;
}

Tree tree_from_file (Str filename) {
    return tree_from_string(string_from_file(filename), filename);
}

} using namespace ayu;

AYU_DESCRIBE(ayu::X::ParseError,
    delegate(base<X::LogicError>()),
    elems(
        elem(&X::ParseError::mess),
        elem(&X::ParseError::filename),
        elem(&X::ParseError::line),
        elem(&X::ParseError::col)
    )
)

#ifndef TAP_DISABLE_TESTS
#include "../../tap/tap.h"
static tap::TestSet tests ("base/ayu/parse", []{
    using namespace tap;
    auto t = [](const char* s, const Tree& t){
        try_is<Tree>([&]{return tree_from_string(s);}, t, cat("yes: "s, s).c_str());
    };
    auto f = [](const char* s){
        throws<X::ParseError>([&]{
            tree_from_string(s);
        }, "no: "s + s);
    };
    t("null", Tree(null));
    t("0", Tree(0));
    t("345", Tree(345));
    t("-44", Tree(-44));
    t("2.5", Tree(2.5));
    t("-4", Tree(-4.0));
    t("1e45", Tree(1e45));
    t("0xdeadbeef00", Tree(0xdeadbeef00));
    t("+0x40", Tree(0x40));
    t("-0x40", Tree(-0x40));
    t("000099", Tree(99));
    t("000", Tree(0));
    f("0.");
    f(".0");
    t("0xdead.beefP30", Tree(0xdead.beefP30));
    t("+0xdead.beefP30", Tree(0xdead.beefP30));
    t("-0xdead.beefP30", Tree(-0xdead.beefP30));
    f("++0");
    f("--0");
    t("+nan", Tree(0.0/0.0));
    t("+inf", Tree(1.0/0.0));
    t("-inf", Tree(-1.0/0.0));
    t("\"\"", Tree(""));
    t("asdf", Tree("asdf"));
    t("\"null\"", Tree("null"));
    t("\"true\"", Tree("true"));
    t("\"false\"", Tree("false"));
    t("[]", Tree(Array{}));
    t("[,,,,,]", Tree(Array{}));
    t("[0 1 foo]", Tree(Array{Tree(0), Tree(1), Tree("foo")}));
    t("{}", Tree(Object{}));
    t("{\"asdf\":\"foo\"}", Tree(Object{Pair{"asdf", Tree("foo")}}));
    t("{\"asdf\":0}", Tree(Object{Pair{"asdf", Tree(0)}}));
    t("{asdf:0}", Tree(Object{Pair{"asdf", Tree(0)}}));
    f("{0:0}");
    t("{a:0 \"null\":1 \"0\":foo}",
        Tree(Object{
            Pair{"a", Tree(0)},
            Pair{"null", Tree(1)},
            Pair{"0", Tree("foo")}
        })
    );
    t("[[0 1] [[2] [3 4]]]",
        Tree(Array{
            Tree(Array{Tree(0), Tree(1)}),
            Tree(Array{
                Tree(Array{Tree(2)}),
                Tree(Array{Tree(3), Tree(4)})
            })
        })
    );
    t("&foo 1", Tree(1));
    t("&foo:1 *foo", Tree(1));
    t("&\"null\":4 *\"null\"", Tree(4));
    t("[&foo 1 *foo]", Tree(Array{Tree(1), Tree(1)}));
    t("[&foo:1 *foo]", Tree(Array{Tree(1)}));
    t("{&key asdf:*key}", Tree(Object{Pair{"asdf", Tree("asdf")}}));
    t("{&borp:\"bump\" *borp:*borp}", Tree(Object{Pair{"bump", Tree("bump")}}));
    t("3 //4", Tree(3));
    t("#", Tree("#"));
    t("#foo", Tree("#foo"));
    f("{&borp:44 *borp:*borp}");
    f("&foo");
    f("&foo:1");
    f("&1 1");
    f("&null 1");
    f("*foo");
    f("4 &foo:4");
    f("&foo *foo");
    f("&foo:*foo 1");
    f("&&a 1");
    f("& a 1");
    f("[+nana]");
    done_testing();
});
#endif
