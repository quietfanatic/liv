#include "parse.h"

#include <cstring>
#include <sstream>
#include <iomanip>

#include "char-cases-internal.h"
#include "compat.h"
#include "describe.h"
#include "print.h"

using namespace std::string_literals;

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
     // of refs (I can't imagine for my use cases having more than 20 or so).
    Object refs;

    Parser (Str s, Str filename = "") :
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
            case EOF: return "<EOF>";
            case ' ': return "<space>";
            default: {
                if (c > 0x20 && c < 0x7e) {
                    return String(1, c);
                }
                else {
                    return "<"s + show_hex_digit((c >> 4) & 0x0f) + show_hex_digit(c & 0xf) + ">";
                }
            }
        }
    }
    X::ParseError error (String&& s) {
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
        return X::ParseError(std::move(s), filename, line, col);
    }

    void skip_comment () {
        p++;  // for the #
        for (;;) switch (look()) {
            case EOF: return;
            case '\n': p++; return;
            default: p++; break;
        }
    }
    void skip_ws () {
        for (;;) switch (look()) {
            case ANY_WS: p++; break;
            case '#': skip_comment(); break;
            default: return;
        }
    }
    void skip_commas () {
        for (;;) switch (look()) {
            case ANY_WS: p++; break;
            case '#': skip_comment(); break;
            case ',': p++; break;
            default: return;
        }
    }

    String got_string () {
        p++;  // for the "
        String r;
        for (;;) switch (look()) {
            case EOF: throw error("String not terminated by end of input");
            case '"': p++; return r;
            case '\\': {
                p++;
                switch (look()) {
                    case EOF: throw error("String not terminated by end of input");
                    case '"': r += '"'; break;
                    case '\\': r += "\\"; break;
                    case '/': r += "/"; break;  // Dunno why this is in json
                    case 'b': r += "\b"; break;
                    case 'f': r += "\f"; break;
                    case 'n': r += "\n"; break;
                    case 'r': r += "\r"; break;
                    case 't': r += "\t"; break;
                    default: throw error("Unrecognized escape sequence \\" + show_char(look()));
                }
                p++;
                break;
            }
            default: r += *p++;
        }
    }

    String got_word () {
        const char* start = p;
        p++;  // For the first character
        for (;;) switch (look()) {
            case ANY_LETTER: case ANY_NUMBER: case ANY_WORD_SYMBOL:
                p++; break;
            case ':': {
                 // Allow :: (for c++ types) or :/ (for urls)
                if (look(1) == ':' || look(1) == '/') {
                    p += 2;
                    break;
                }
                else return String(start, p - start);
            }
            case '"': {
                throw error("\" cannot occur inside a word (are you missing the first \"?)");
            }
            case ANY_RESERVED_SYMBOL: {
                throw error(*p + " is a reserved symbol and can't be used outside of strings.");
            }
            default: return String(start, p - start);
        }
    }

     // This is so horrible, wish I had from_chars
     // TODO: support hexadecimal
    Tree got_number () {
        String word = got_word();
        if (word == "+nan" || word == "-nan") return Tree(0.0/0.0);
        else if (word == "+inf") return Tree(1.0/0.0);
        else if (word == "-inf") return Tree(-1.0/0.0);
         // Squeeze out _s in place
        auto oi = word.begin();
        for (auto ii = word.begin(); ii != word.end(); ii++) {
            if (*ii != '_') {
                *oi++ = *ii;
            }
        }
        *oi = 0;
         // First try reading as int
        int64 i;
        std::istringstream is (word);
        is.imbue(std::locale::classic());
        is >> i;
        if (word[is.tellg()] == 0) return Tree(i);
         // Didn't read the whole thing?  Try reading as double.
        double d;
        std::istringstream ds (word);
        ds.imbue(std::locale::classic());
        ds >> d;
        if (word[ds.tellg()] == 0) return Tree(d);
        throw error("Malformed numeric value");
    }

//    Tree parse_heredoc () {
//        p++;  // for the <
//        if (look() != '<') throw error("< isn't followed by another < for a heredoc");
//        p++;
//        String terminator = parse_ident("heredoc delimiter string after <<");
//        while (look() == ' ' || look() == '\t' || look() == '\r') p++;
//        if (look() != '\n') throw error("Extra stuff after <<" + terminator + " before end of line");
//        p++;
//        String got = "";
//        while (1) {
//            String ind = "";
//            while (look() == ' ' || look() == '\t') {
//                ind += look(); got += look(); p++;
//            }
//            if (p + terminator.size() > end) throw error("Ran into end of document before " + terminator);
//            if (0==strncmp(p, terminator.c_str(), terminator.size())) {
//                String ret;
//                usize p1 = 0;
//                usize p2 = got.find('\n');
//                while (p2 != String::npos) {
//                    p2 += 1;
//                    if (0==strncmp(got.c_str() + p1, ind.c_str(), ind.size())) {
//                        p1 += ind.size();
//                    }
//                    ret += got.substr(p1, p2 - p1);
//                    p1 = p2;
//                    p2 = got.find('\n', p2);
//                }
//                p += terminator.size();
//                return Tree(ret);
//            }
//            while (look() != '\n') {
//                got += look(); p++;
//            }
//            got += look(); p++;
//        }
//    }

    Array got_array () {
        Array a;
        p++;  // for the [
        for (;;) {
            skip_commas();
            switch (look()) {
                case EOF: throw error("Array not terminated");
                case ':': throw error("Cannot have : in an array");
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
                case EOF: throw error("Object not terminated");
                case ':': throw error("Missing key before : in object");
                case '}': p++; return o;
                default: break;
            }
            Tree key = parse_term();
            if (key.form() != STRING) {
                throw error("Can't use non-string " + tree_to_string(key) + " as key in object");
            }
            skip_ws();
            switch (look()) {
                case EOF: throw error("Object not terminated");
                case ':': p++; break;
                case ANY_RESERVED_SYMBOL: {
                    throw error(*p + " is a reserved symbol and can't be used outside of strings."s);
                }
                default: throw error("Missing : after name in object");
            }
            skip_ws();
            switch (look()) {
                case ',':
                case '}': throw error("Missing value after : in object");
                default: {
                    o.emplace_back(String(key), parse_term());
                    break;
                }
            }
        }
        return o;
    }

    void add_ref (Str name, Tree value) {
        for (auto& p : refs) {
            if (p.first == name) {
                throw error("Duplicate declaration of ref &"
                    + tree_to_string(Tree(name))
                );
            }
        }
        refs.emplace_back(String(name), std::move(value));
    }
    const Tree& get_ref (Str name) {
        for (auto& p : refs) {
            if (p.first == name) return p.second;
        }
        throw error("Unknown ref *" +
            tree_to_string(Tree(name))
        );
    }

    Tree got_decl () {
        p++;  // for the &
        switch (look()) {
            case ANY_LETTER:
            case '_':
            case '"': break;
            default: throw error("Expected ref name after &");
        }
        Tree name = parse_term();
        if (name.form() != STRING) {
            throw error("Can't use non-string " + tree_to_string(name) + " as ref name");
        }
        skip_ws();
        switch (look()) {
            case ':': {
                p++;
                skip_ws();
                add_ref(Str(name), parse_term());
                skip_commas();
                return parse_term();
            }
            default: {
                Tree value = parse_term();
                add_ref(Str(name), value);
                return value;
            }
        }
    }

    Tree got_ref () {
        p++;  // for the *
        switch (look()) {
            case ANY_LETTER:
            case '_':
            case '"': break;
            default: throw error("Expected ref name after *");
        }
        Tree name = parse_term();
        if (name.form() != STRING) {
            throw error("Can't use non-string " + tree_to_string(name) + " as ref name");
        }
        return get_ref(Str(name));
    }

    Tree parse_term () {
        switch (look()) {
            case EOF: throw error("Expected term but ran into end of document");
            case ANY_LETTER:
            case '_': {
                String word = got_word();
                if (word == "null") return Tree(null);
                else if (word == "true") return Tree(true);
                else if (word == "false") return Tree(false);
                else return Tree(word);
            }

            case ANY_NUMBER:
            case '+':
            case '-':
            case '.': return got_number();

            case '"': return Tree(got_string());
            case '[': return Tree(got_array());
            case '{': return Tree(got_object());

            case '&': return got_decl();
            case '*': return got_ref();

            case ':':
            case ',':
            case ']':
            case '}': throw error("Unexpected "s + *p);
            case ANY_RESERVED_SYMBOL:
                throw error(*p + " is a reserved symbol and can't be used outside of strings."s);
            default: throw error("Unrecognized character "s + *p);
        }
    }

    Tree parse () {
        skip_ws();
        Tree r = parse_term();
        for (;;) switch (look()) {
            case EOF: return r;
            case ANY_WS: p++; continue;
            case '#': skip_comment(); continue;
            default: throw error("Extra stuff at end of document");
        }
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
    fread(const_cast<char*>(r.data()), 1, size, f);

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
    elems(
        elem(&X::ParseError::mess),
        elem(&X::ParseError::filename),
        elem(&X::ParseError::line),
        elem(&X::ParseError::col)
    )
)

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"
static tap::TestSet tests ("base/ayu/parse", []{
    using namespace tap;
    auto t = [](const char* s, const Tree& t){
        try_is<Tree>([&]{return tree_from_string(s);}, t, "yes: "s + s);
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
    done_testing();
});
#endif
