#include "../parse.h"

#include <cstring>
#include <charconv>
#include <limits>

#include "../compat.h"
#include "../describe.h"
#include "../print.h"
#include "char-cases-private.h"
#include "tree-private.h"

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
                if (look(1) == '/') {
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
                if (look(1) == '/') {
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
        Str word = got_word();
         // Detect special numbers
        if (word == "+nan"sv) {
            return Tree(std::numeric_limits<double>::quiet_NaN());
        }
        if (word == "+inf"sv) {
            return Tree(std::numeric_limits<double>::infinity());
        }
        if (word == "-inf"sv) {
            return Tree(-std::numeric_limits<double>::infinity());
        }
         // Detect sign
        bool minus = false;
        switch (word[0]) {
            case '+': {
                word = word.substr(1);
                if (word.empty() || !std::isdigit(word[0])) {
                    throw error("Malformed number"sv);
                }
                break;
            }
            case '-': {
                minus = true;
                word = word.substr(1);
                if (word.empty() || !std::isdigit(word[0])) {
                    throw error("Malformed number"sv);
                }
                break;
            }
            default: break;
        }
         // Detect hex prefix
        bool hex = false;
        if (word.size() >= 2 && word[0] == '0'
         && (word[1] == 'x' || word[1] == 'X')
        ) {
            hex = true;
            word = word.substr(2);
        }
         // Try integer
        {
            int64 integer;
            auto [ptr, ec] = std::from_chars(
                word.begin(), word.end(), integer, hex ? 16 : 10
            );
            if (ptr == word.begin()) {
                 // If the integer parse failed, the float parse will also fail.
                throw error("Malformed number"sv);
            }
            else if (ptr == word.end()) {
                return Tree(minus ? -integer : integer);
            }
             // Forbid . without a digit after
            else if (ptr < word.end() && ptr[0] == '.') {
                if (ptr == word.end() + 1 ||
                    (hex ? !std::isxdigit(ptr[1]) : !std::isdigit(ptr[1]))
                ) {
                    throw error("Number cannot end with a ."sv);
                }
            }
        }
         // Integer parse didn't take the whole word, try float parse
        {
            double floating;
            auto [ptr, ec] = std::from_chars(
                word.begin(), word.end(), floating,
                hex ? std::chars_format::hex
                    : std::chars_format::general
            );
            if (ptr == word.begin()) {
                 // Shouldn't happen?
                throw error("Malformed number"sv);
            }
            else if (ptr == word.end()) {
                return Tree(minus ? -floating : floating);
            }
            else {
                throw error("Junk at end of number"sv);
            }
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
            case '}': throw error("Unexpected "sv, *p);
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
    auto y = [](const char* s, const Tree& t){
        try_is<Tree>([&]{return tree_from_string(s);}, t, cat("yes: "s, s).c_str());
    };
    auto n = [](const char* s){
        throws<X::ParseError>([&]{
            tree_from_string(s);
        }, "no: "s + s);
    };
    y("null", Tree(null));
    y("0", Tree(0));
    y("345", Tree(345));
    y("-44", Tree(-44));
    y("2.5", Tree(2.5));
    y("-4", Tree(-4.0));
    y("1e45", Tree(1e45));
    y("0xdeadbeef00", Tree(0xdeadbeef00));
    y("+0x40", Tree(0x40));
    y("-0x40", Tree(-0x40));
    y("000099", Tree(99));
    y("000", Tree(0));
    n("0.");
    n(".0");
    n("0.e4");
    y("0xdead.beefP30", Tree(0xdead.beefP30));
    y("+0xdead.beefP30", Tree(0xdead.beefP30));
    y("-0xdead.beefP30", Tree(-0xdead.beefP30));
    n("++0");
    n("--0");
    y("+nan", Tree(0.0/0.0));
    y("+inf", Tree(1.0/0.0));
    y("-inf", Tree(-1.0/0.0));
    y("\"\"", Tree(""));
    y("asdf", Tree("asdf"));
    y("\"null\"", Tree("null"));
    y("\"true\"", Tree("true"));
    y("\"false\"", Tree("false"));
    y("[]", Tree(Array{}));
    y("[,,,,,]", Tree(Array{}));
    y("[0 1 foo]", Tree(Array{Tree(0), Tree(1), Tree("foo")}));
    y("{}", Tree(Object{}));
    y("{\"asdf\":\"foo\"}", Tree(Object{Pair{"asdf", Tree("foo")}}));
    y("{\"asdf\":0}", Tree(Object{Pair{"asdf", Tree(0)}}));
    y("{asdf:0}", Tree(Object{Pair{"asdf", Tree(0)}}));
    n("{0:0}");
    y("{a:0 \"null\":1 \"0\":foo}",
        Tree(Object{
            Pair{"a", Tree(0)},
            Pair{"null", Tree(1)},
            Pair{"0", Tree("foo")}
        })
    );
    y("[[0 1] [[2] [3 4]]]",
        Tree(Array{
            Tree(Array{Tree(0), Tree(1)}),
            Tree(Array{
                Tree(Array{Tree(2)}),
                Tree(Array{Tree(3), Tree(4)})
            })
        })
    );
    y("&foo 1", Tree(1));
    y("&foo:1 *foo", Tree(1));
    y("&\"null\":4 *\"null\"", Tree(4));
    y("[&foo 1 *foo]", Tree(Array{Tree(1), Tree(1)}));
    y("[&foo:1 *foo]", Tree(Array{Tree(1)}));
    y("{&key asdf:*key}", Tree(Object{Pair{"asdf", Tree("asdf")}}));
    y("{&borp:\"bump\" *borp:*borp}", Tree(Object{Pair{"bump", Tree("bump")}}));
    y("3 //4", Tree(3));
    y("#", Tree("#"));
    y("#foo", Tree("#foo"));
    n("{&borp:44 *borp:*borp}");
    n("&foo");
    n("&foo:1");
    n("&1 1");
    n("&null 1");
    n("*foo");
    n("4 &foo:4");
    n("&foo *foo");
    n("&foo:*foo 1");
    n("&&a 1");
    n("& a 1");
    n("[+nana]");
    done_testing();
});
#endif
