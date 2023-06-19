#include "iri.h"

#include <cassert>

namespace iri {

#define IRI_UPPERCASE \
         'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': \
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': \
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': \
    case 'V': case 'W': case 'X': case 'Y': case 'Z'
#define IRI_LOWERCASE \
         'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': \
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': \
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': \
    case 'v': case 'w': case 'x': case 'y': case 'z'
#define IRI_DIGIT \
         '0': case '1': case '2': case '3': case '4': \
    case '5': case '6': case '7': case '8': case '9'
#define IRI_UPPERHEX \
         'A': case 'B': case 'C': case 'D': case 'E': case 'F'
#define IRI_LOWERHEX \
         'a': case 'b': case 'c': case 'd': case 'e': case 'f'
#define IRI_GENDELIM \
         ':': case '/': case '?': case '#': case '[': case ']': case '@'
#define IRI_SUBDELIM \
         '!': case '$': case '&': case '\'': case '(': case ')': \
    case '*': case '+': case ',': case ';': case '='
#define IRI_UNRESERVED_SYMBOL \
         '-': case '.': case '_': case '~'
#define IRI_FORBIDDEN \
         0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: \
    case 0x06: case 0x07: case 0x08: case 0x09: case 0x0a: case 0x0b: \
    case 0x0c: case 0x0d: case 0x0e: case 0x0f: \
    case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: \
    case 0x16: case 0x17: case 0x18: case 0x19: case 0x1a: case 0x1b: \
    case 0x1c: case 0x1d: case 0x1e: case 0x1f: \
    case 0x20: case 0x7f
#define IRI_IFFY \
         '<': case '>': case '"': case '{': case '}': case '|': case '\\': \
    case '^': case '`'
#define IRI_UTF8_HIGH \
         char(0x80): case char(0x81): case char(0x82): case char(0x83): \
    case char(0x84): case char(0x85): case char(0x86): case char(0x87): \
    case char(0x88): case char(0x89): case char(0x8a): case char(0x8b): \
    case char(0x8c): case char(0x8d): case char(0x8e): case char(0x8f): \
    case char(0x90): case char(0x91): case char(0x92): case char(0x93): \
    case char(0x94): case char(0x95): case char(0x96): case char(0x97): \
    case char(0x98): case char(0x99): case char(0x9a): case char(0x9b): \
    case char(0x9c): case char(0x9d): case char(0x9e): case char(0x9f): \
    case char(0xa0): case char(0xa1): case char(0xa2): case char(0xa3): \
    case char(0xa4): case char(0xa5): case char(0xa6): case char(0xa7): \
    case char(0xa8): case char(0xa9): case char(0xaa): case char(0xab): \
    case char(0xac): case char(0xad): case char(0xae): case char(0xaf): \
    case char(0xb0): case char(0xb1): case char(0xb2): case char(0xb3): \
    case char(0xb4): case char(0xb5): case char(0xb6): case char(0xb7): \
    case char(0xb8): case char(0xb9): case char(0xba): case char(0xbb): \
    case char(0xbc): case char(0xbd): case char(0xbe): case char(0xbf): \
    case char(0xc0): case char(0xc1): case char(0xc2): case char(0xc3): \
    case char(0xc4): case char(0xc5): case char(0xc6): case char(0xc7): \
    case char(0xc8): case char(0xc9): case char(0xca): case char(0xcb): \
    case char(0xcc): case char(0xcd): case char(0xce): case char(0xcf): \
    case char(0xd0): case char(0xd1): case char(0xd2): case char(0xd3): \
    case char(0xd4): case char(0xd5): case char(0xd6): case char(0xd7): \
    case char(0xd8): case char(0xd9): case char(0xda): case char(0xdb): \
    case char(0xdc): case char(0xdd): case char(0xde): case char(0xdf): \
    case char(0xe0): case char(0xe1): case char(0xe2): case char(0xe3): \
    case char(0xe4): case char(0xe5): case char(0xe6): case char(0xe7): \
    case char(0xe8): case char(0xe9): case char(0xea): case char(0xeb): \
    case char(0xec): case char(0xed): case char(0xee): case char(0xef): \
    case char(0xf0): case char(0xf1): case char(0xf2): case char(0xf3): \
    case char(0xf4): case char(0xf5): case char(0xf6): case char(0xf7): \
    case char(0xf8): case char(0xf9): case char(0xfa): case char(0xfb): \
    case char(0xfc): case char(0xfd): case char(0xfe): case char(0xff)
#define IRI_UNRESERVED \
         IRI_UPPERCASE: case IRI_LOWERCASE: case IRI_DIGIT: \
    case IRI_UNRESERVED_SYMBOL: case IRI_UTF8_HIGH

std::string encode (OldStr input) {
    std::string r;
    r.reserve(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        switch (input[i]) {
            case IRI_GENDELIM: case IRI_SUBDELIM:
            case IRI_FORBIDDEN: case IRI_IFFY:
            case '%': {
                uint8 high = uint8(input[i]) >> 4;
                uint8 low = uint8(input[i]) & 0xf;
                r += '%';
                r += high >= 10 ? high - 10 + 'A' : high + '0';
                r += low >= 10 ? low - 10 + 'A' : low + '0';
                break;
            }
            default: r += input[i]; break;
        }
    }
    return r;
}

std::string decode (OldStr input) {
    std::string r;
    r.reserve(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        if (input[i] == '%' && i + 2 < input.size()) {
            uint8 byte = 0;
            for (int j = 1; j <= 2; j++) {
                byte <<= 4;
                switch (input[i+j]) {
                    case IRI_DIGIT: byte |= input[i+j] - '0'; break;
                    case IRI_UPPERHEX: byte |= input[i+j] - 'A' + 10; break;
                    case IRI_LOWERHEX: byte |= input[i+j] - 'a' + 10; break;
                    default: return ""; // Invalid
                }
            }
            r += char(byte);
            i += 2;
        }
        else r += input[i];
    }
    return r;
}

IRIRelativity classify_reference (OldStr ref) {
    if (ref.size() == 0) return SCHEME;
    switch (ref[0]) {
        case ':': return SCHEME;
        case '/':
            if (ref.size() > 1 && ref[1] == '/') {
                return AUTHORITY;
            }
            else return PATHABSOLUTE;
        case '?': return QUERY;
        case '#': return FRAGMENT;
        default: break;
    }
    for (size_t i = 1; i < ref.size(); i++) {
        switch (ref[i]) {
            case ':': return SCHEME;
            case '/': case '?': case '#':
                return PATHRELATIVE;
            default: break;
        }
    }
    return PATHRELATIVE;
}

IRI::IRI (OldStr input, const IRI& base) :
    colon_(0),
    path_(0),
    question_(0),
    hash_(0)
{
     // Reject absurdly large input
    if (input.size() > maximum_length) return;
    uint32 i = 0;
    uint32 colon = 0;
    uint32 path = 0;
    uint32 question = 0;
    uint32 hash = 0;
     // A few helpers...
     // Make this IRI invalid
    auto fail = [&]{};
     // Decode (maybe) a % sequence
    auto write_percent = [&](char c){
        uint8 high = uint8(c) >> 4;
        uint8 low = c & 0xf;
        spec_ += '%';
        spec_ += (high >= 10 ? high - 10 + 'A' : high + '0');
        spec_ += (low >= 10 ? low - 10 + 'A' : low + '0');
    };
    auto read_percent = [&]{
        if (i + 3 >= input.size()) return false;
        uint8 byte = 0;
        for (int j = 1; j < 3; j++) {
            byte <<= 4;
            switch (input[i+j]) {
                case IRI_DIGIT: byte |= input[i+j] - '0'; break;
                case IRI_UPPERHEX: byte |= input[i+j] - 'A' + 10; break;
                case IRI_LOWERHEX: byte |= input[i+j] - 'a' + 10; break;
                default: return false;
            }
        }
        switch (byte) {
            case IRI_GENDELIM: case IRI_SUBDELIM:
            case IRI_FORBIDDEN: case IRI_IFFY:
                write_percent(byte); break;
            default: spec_ += char(byte); break;
        }
        i += 3; return true;
    };
     // Now start parsing...wait stop.
     // If we've been given a relative reference, we can skip some parsing
    switch (classify_reference(input)) {
        case SCHEME: {
             // Optimize for the case that the input won't be altered
            spec_.reserve(input.size());
            goto parse_scheme;
        }
        case AUTHORITY: {
            OldStr prefix = base.spec_with_scheme();
            if (!prefix.size()) goto fail;
            spec_.reserve(prefix.size() + input.size());
            spec_ += prefix;
            colon = base.colon_;
            assert(colon + 1 == spec_.size());
            goto parse_authority;
        }
        case PATHABSOLUTE: {
            if (!base.is_hierarchical()) goto fail;
            OldStr prefix = base.spec_with_origin();
            assert(prefix.size());
            spec_.reserve(prefix.size() + input.size());
            spec_ += prefix;
            colon = base.colon_;
            path = base.path_;
            assert(path == spec_.size());
            goto parse_path;
        }
        case PATHRELATIVE: {
            if (!base.is_hierarchical()) goto fail;
            OldStr prefix = base.spec_without_filename();
            assert(prefix.size());
            spec_.reserve(prefix.size() + input.size());
            spec_ += prefix;
            colon = base.colon_;
            path = base.path_;
            assert(path < spec_.size());
            goto parse_path;
        }
        case QUERY: {
            OldStr prefix = base.spec_without_query();
            if (!prefix.size()) goto fail;
            spec_.reserve(prefix.size() + input.size());
            spec_ += prefix;
            assert(input[i] == '?');
            spec_ += '?'; i++;
            colon = base.colon_;
            path = base.path_;
            question = base.question_;
            assert(question + 1 == spec_.size());
            goto parse_query;
        }
        case FRAGMENT: {
            OldStr prefix = base.spec_without_fragment();
            if (!prefix.size()) goto fail;
            spec_.reserve(prefix.size() + input.size());
            spec_ += prefix;
            assert(input[i] == '#');
            spec_ += '#'; i++;
            colon = base.colon_;
            path = base.path_;
            question = base.question_;
            hash = base.hash_;
            assert(hash + 1 == spec_.size());
            goto parse_fragment;
        }
        default: assert(false);
    }
     // Okay NOW start parsing.

    parse_scheme: while (i < input.size()) {
        switch (input[i]) {
            case IRI_UPPERCASE:
                 // Canonicalize to lowercase
                spec_ += input[i++] - 'A' + 'a';
                break;
            case IRI_LOWERCASE:
                spec_ += input[i++];
                break;
            case IRI_DIGIT: case '+': case '-': case '.':
                if (i == 0) goto fail;
                else spec_ += input[i++];
                break;
            case ':':
                if (i == 0) goto fail;
                colon = spec_.size();
                spec_ += input[i++];
                goto parse_authority;
            default: goto fail;
        }
    }
    goto fail;

    parse_authority:
    if (i + 1 < input.size() && input[i] == '/' && input[i+1] == '/') {
        spec_ += input[i++]; spec_ += input[i++];
        while (i < input.size()) {
            switch (input[i]) {
                case IRI_UPPERCASE:
                     // Canonicalize to lowercase
                    spec_ += input[i++] - 'A' + 'a';
                    break;
                case IRI_LOWERCASE: case IRI_DIGIT:
                case IRI_UNRESERVED_SYMBOL:
                case IRI_UTF8_HIGH:
                case IRI_SUBDELIM:
                case ':': case '[': case ']': case '@':
                    spec_ += input[i++];
                    break;
                case '/':
                    path = spec_.size();
                    goto parse_path;
                case '?':
                    path = question = spec_.size();
                    spec_ += input[i++];
                    goto parse_query;
                case '#':
                    path = question = hash = spec_.size();
                    spec_ += input[i++];
                    goto parse_fragment;
                case '%':
                    if (!read_percent()) goto fail;
                    break;
                case IRI_IFFY:
                    write_percent(input[i++]);
                    break;
                default: goto fail;
            }
        }
        path = question = hash = spec_.size();
        goto done;
    }
    else {
         // No authority
        path = i;
    }

    parse_path:
     // We may or may not have the / already.  Kind of an awkward condition
     // but it's the less confusing than making sure one or the other case
     // is always true.
    if ((path < spec_.size() && spec_[path] == '/')
     || (path == spec_.size() && input[i] == '/')) {
         // Canonicalize
        while (i < input.size()) {
            switch (input[i]) {
                case IRI_UPPERCASE: case IRI_LOWERCASE:
                case IRI_DIGIT: case IRI_SUBDELIM:
                case IRI_UTF8_HIGH:
                case '-': case '_': case '~': case ':': case '@':
                    spec_ += input[i++];
                    break;
                case '/': {
                     // Eliminate duplicate /
                    if (spec_.back() != '/') {
                        spec_ += input[i++];
                    }
                    else i++;
                    break;
                }
                case '.': {
                    if (spec_.back() == '/') {
                        if (i+1 < input.size() && input[i+1] == '.') {
                            if (i+2 == input.size()
                                || input[i+2] == '/'
                                || input[i+2] == '?'
                                || input[i+2] == '#'
                            ) {
                                 // Got a .. so pop off last segment
                                if (spec_.size() > path + 1) {
                                    spec_.pop_back(); // last slash
                                    while (spec_.back() != '/') {
                                        spec_.pop_back();
                                    }
                                    i += 2; break;
                                }
                                else { fail(); return; }
                            }
                        }
                        else if (i+1 == input.size()
                            || input[i+1] == '/'
                            || input[i+1] == '?'
                            || input[i+1] == '#'
                        ) {
                             // Go a . so ignore it
                            i++; break;
                        }
                    }
                    spec_ += input[i++];
                    break;
                }
                case '?':
                    question = spec_.size();
                    spec_ += input[i++];
                    goto parse_query;
                case '#':
                    question = hash = spec_.size();
                    spec_ += input[i++];
                    goto parse_fragment;
                case '%':
                    if (!read_percent()) goto fail;
                    break;
                case IRI_IFFY:
                    write_percent(input[i++]);
                    break;
                default: goto fail;
            }
        }
        question = hash = spec_.size();
        goto done;
    }
    else {
         // Don't canonicalize
        while (i < input.size()) {
            switch (input[i]) {
                case IRI_UPPERCASE: case IRI_LOWERCASE:
                case IRI_DIGIT: case IRI_SUBDELIM:
                case IRI_UTF8_HIGH:
                case '-': case '_': case '~':
                case ':': case '@': case '/':
                    spec_ += input[i++];
                    break;
                case '?':
                    question = spec_.size();
                    spec_ += input[i++];
                    goto parse_query;
                case '#':
                    question = hash = spec_.size();
                    spec_ += input[i++];
                    goto parse_fragment;
                case '%':
                    if (!read_percent()) goto fail;
                    break;
                case IRI_IFFY:
                    write_percent(input[i]);
                    i++; break;
                default: goto fail;
            }
        }
        question = hash = spec_.size();
        goto done;
    }
    parse_query: while (i < input.size()) {
        switch (input[i]) {
            case IRI_UNRESERVED: case IRI_SUBDELIM:
            case ':': case '@': case '/': case '?':
                spec_ += input[i++];
                break;
            case '#':
                hash = spec_.size();
                spec_ += input[i++];
                goto parse_fragment;
            case '%':
                if (!read_percent()) goto fail;
                break;
            case IRI_IFFY:
                write_percent(input[i++]);
                break;
            default: goto fail;
        }
    }
    hash = spec_.size();
    goto done;

    parse_fragment: while (i < input.size()) {
         // Note that a second # is not allowed.  If that happens, it's likely
         // that there is a nested URL with an unescaped fragment, and in that
         // case it's ambiguous how to parse it, so we won't try.
        switch (input[i]) {
            case IRI_UNRESERVED: case IRI_SUBDELIM:
            case ':': case '@': case '/': case '?':
                spec_ += input[i++];
                break;
            case '%':
                if (!read_percent()) goto fail;
                break;
            case IRI_IFFY:
                write_percent(input[i++]);
                break;
            default: goto fail;
        }
    }
    goto done;

    done: {
        if (spec_.size() > maximum_length) goto fail;
        assert(colon < path);
        assert(colon + 2 != path);
        assert(path <= question);
        assert(question <= hash);
        assert(hash <= spec_.size());
        colon_ = colon;
        path_ = path;
        question_ = question;
        hash_ = hash;
        return;
    }
    fail: {
        spec_ += input.substr(i);
        return;
    }
}

IRI::IRI (std::string&& spec, uint16 c, uint16 p, uint16 q, uint16 h) :
    spec_(std::move(spec)), colon_(c), path_(p), question_(q), hash_(h)
{ }

IRI::IRI (const IRI& o) = default;
IRI::IRI (IRI&& o) :
    spec_(std::move(o.spec_)),
    colon_(o.colon_),
    path_(o.path_),
    question_(o.question_),
    hash_(o.hash_)
{ o.colon_ = o.path_ = o.question_ = o.hash_ = 0; }
IRI& IRI::operator = (const IRI& o) {
    this->~IRI();
    new (this) IRI(o);
    return *this;
}
IRI& IRI::operator = (IRI&& o) {
    this->~IRI();
    new (this) IRI(std::move(o));
    return *this;
}

bool IRI::is_valid () const { return colon_; }
bool IRI::is_empty () const { return spec_.empty(); }
IRI::operator bool () const { return colon_; }

static const std::string empty = "";

const std::string& IRI::spec () const {
    if (colon_) return spec_;
    else return empty;
}
const std::string& IRI::possibly_invalid_spec () const {
    return spec_;
}

std::string IRI::move_spec () {
    if (colon_) return empty;
    std::string r = std::move(spec_);
    *this = IRI();
    return r;
}
std::string IRI::move_possibly_invalid_spec () {
    std::string r = std::move(spec_);
    *this = IRI();
    return r;
}

std::string IRI::spec_relative_to (const IRI& base) const {
    if (!*this || !base) {
        return "";
    }
    else if (has_authority() != base.has_authority()
          || !is_hierarchical() || !base.is_hierarchical()
          || scheme() != base.scheme()
    ) {
        return spec();
    }
    else if (has_authority() && authority() != base.authority()) {
        return std::string(&spec_[colon_ + 1], spec_.size() - (colon_ + 1));
    }
    else if ((!has_query() && !has_fragment())
           || path() != base.path()
    ) {
         // Pulling apart path is NYI
        return std::string(&spec_[path_], spec_.size() - path_);
    }
    else if (has_query() && (!has_fragment() || query() != base.query())) {
        return std::string(&spec_[question_], spec_.size() - question_);
    }
    else {
        return std::string(&spec_[hash_], spec_.size() - (hash_));
    }
}

bool IRI::has_scheme () const { return colon_; }
bool IRI::has_authority () const { return path_ >= colon_ + 3; }
bool IRI::has_path () const { return question_ > path_; }
bool IRI::has_query () const { return hash_ > question_; }
bool IRI::has_fragment () const { return hash_ && spec_.size() > hash_; }

bool IRI::is_hierarchical () const {
    return has_path() && spec_[path_] == '/';
}

OldStr IRI::scheme () const {
    return OldStr(&spec_[0], colon_);
}
OldStr IRI::authority () const {
    return has_authority()
        ? OldStr(&spec_[colon_ + 3], path_ - (colon_ + 3))
        : OldStr();
}
OldStr IRI::path () const {
    return OldStr(&spec_[path_], question_ - path_);
}
OldStr IRI::query () const {
    return has_query()
        ? OldStr(&spec_[question_ + 1], hash_ - (question_ + 1))
        : OldStr();
}
OldStr IRI::fragment () const {
    return has_fragment()
        ? OldStr(&spec_[hash_ + 1], spec_.size() - (hash_ + 1))
        : OldStr();
}

IRI IRI::iri_with_scheme () const {
    if (has_scheme()) return IRI(
        std::string(&spec_[0], colon_+1),
        colon_, colon_+1, colon_+1, colon_+1
    );
    else return IRI();
}
IRI IRI::iri_with_origin () const {
    return IRI(
        std::string(spec_with_origin()),
        colon_, path_, path_, path_
    );
}
IRI IRI::iri_without_filename () const {
    if (is_hierarchical()) {
        uint32 i = question_;
        while (spec_[i] != '/') i--;
        return IRI(
            std::string(&spec_[0], i+1),
            colon_, path_, i+1, i+1
        );
    }
    else return IRI();
}
IRI IRI::iri_without_query () const {
    return IRI(
        std::string(&spec_[0], hash_),
        colon_, path_, question_, question_
    );
}
IRI IRI::iri_without_fragment () const {
    return IRI(
        std::string(&spec_[0], hash_),
        colon_, path_, question_, hash_
    );
}

OldStr IRI::spec_with_scheme () const {
    return has_scheme()
        ? OldStr(&spec_[0], colon_ + 1)
        : OldStr();
}
OldStr IRI::spec_with_origin () const {
    return has_authority()
        ? OldStr(&spec_[0], path_)
        : colon_
            ? OldStr(&spec_[0], colon_ + 1)
            : OldStr();
}
OldStr IRI::spec_without_filename () const {
    if (is_hierarchical()) {
        uint32 i = question_;
        while (spec_[i] != '/') i--;
        return OldStr(&spec_[0], i+1);
    }
    else {
        return OldStr(&spec_[0], question_);
    }
}
OldStr IRI::spec_without_query () const {
    return OldStr(&spec_[0], question_);
}
OldStr IRI::spec_without_fragment () const {
    return OldStr(&spec_[0], hash_);
}

OldStr IRI::path_without_filename () const {
    if (is_hierarchical()) {
        uint32 i = question_;
        while (spec_[i] != '/') i--;
        return OldStr(&spec_[path_], i+1);
    }
    else {
        return path();
    }
}

IRI::~IRI () { }

} using namespace iri;

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"

namespace iri::test {

struct TestCase {
    OldStr i = "";
    OldStr b = "";
    OldStr s = "";
    OldStr a = "";
    OldStr p = "";
    OldStr q = "";
    OldStr f = "";
};

 // TODO: Add a LOT more tests, this isn't nearly enough.
constexpr TestCase cases [] = {
    {.i = ""},
    {.i = "foo:", .s = "foo"},
    {.i = "foo:/", .s = "foo", .p = "/"},
    {.i = "foo://", .s = "foo", .a = ""},
    {.i = "foo:bar", .s = "foo", .p = "bar"},
    {.i = "foo:/bar", .s = "foo", .p = "/bar"},
    {.i = "foo://bar", .s = "foo", .a = "bar"},
    {.i = "foo://bar/", .s = "foo", .a = "bar", .p = "/"},
    {.i = "foo://bar/baz", .s = "foo", .a = "bar", .p = "/baz"},
    {.i = "foo:?bar", .s = "foo", .q = "bar"},
    {.i = "foo:#bar", .s = "foo", .f = "bar"},
    {.i = "foo"},
    {.i = "foo::", .s = "foo", .p = ":"},
    {.i = "Foo-b+aR://BAR", .s = "foo-b+ar", .a = "bar"},
    {.i = "foo://bar/baz?qux#bap", .s = "foo", .a = "bar", .p = "/baz", .q = "qux", .f = "bap"},
    {.i = "asdf", .b = "foo:bar"},
    {.i = "asdf", .b = "foo:/bar/baz", .s = "foo", .p = "/bar/asdf"},
    {.i = "/asdf", .b = "foo:/bar/baz", .s = "foo", .p = "/asdf"},
    {.i = "../asdf", .b = "foo:/bar/baz", .s = "foo", .p = "/asdf"},
    {.i = "..", .b = "foo:/bar/baz", .s = "foo", .p = "/"},
    {.i = ".", .b = "foo:/bar/baz", .s = "foo", .p = "/bar/"},
    {.i = ".", .b = "foo:/bar/baz/", .s = "foo", .p = "/bar/baz/"},
    {.i = "..", .b = "foo:/bar"},
    {.i = "../..", .b = "foo:/bar/baz/qux/bap", .s = "foo", .p = "/bar/"},
    {.i = "foo://bar/.."},
    {.i = "foo:/bar/baz/..", .s = "foo", .p = "/bar/"},
    {.i = "?bar", .b = "foo:", .s = "foo", .q = "bar"},
    {.i = "#bar", .b = "foo:", .s = "foo", .f = "bar"},
    {.i = "?bar", .b = "foo:?baz#qux", .s = "foo", .q = "bar"},
    {.i = "#bar", .b = "foo:?baz#qux", .s = "foo", .q = "baz", .f = "bar"},
    {.i = "foo:/ユニコード", .s = "foo", .p = "/ユニコード"},
    {.i = "foo://ユ/ニ?コー#ド", .s = "foo", .a = "ユ", .p = "/ニ", .q = "コー", .f = "ド"},
    {.i = "ayu-test:/#bar/1/bu%2Fp//33/0/'3/''/'//", .s = "ayu-test", .p = "/", .f = "bar/1/bu%2Fp//33/0/'3/''/'//"},
};
constexpr auto n_cases = sizeof(cases) / sizeof(cases[0]);

} // namespace iri::test

static tap::TestSet tests ("base/iri/iri", []{
    using namespace tap;
    using namespace iri::test;
    IRI empty;
    ok(!empty.is_valid(), "!empty.is_valid()");
    ok(empty.is_empty(), "empty.is_empty()");
    ok(!empty, "!empty");
    for (uint32 i = 0; i < n_cases; i++) {
        IRI iri (cases[i].i, IRI(cases[i].b));
        is(iri.scheme(), cases[i].s,
            std::string(cases[i].i) + " ("
            + std::string(cases[i].b) + ") SCHEME = "
            + std::string(cases[i].s)
        );
        is(iri.authority(), cases[i].a,
            std::string(cases[i].i) + " ("
            + std::string(cases[i].b) + ") AUTHORITY = "
            + std::string(cases[i].a)
        );
        is(iri.path(), cases[i].p,
            std::string(cases[i].i) + " ("
            + std::string(cases[i].b) + ") PATH = "
            + std::string(cases[i].p)
        );
        is(iri.query(), cases[i].q,
            std::string(cases[i].i) + " ("
            + std::string(cases[i].b) + ") QUERY = "
            + std::string(cases[i].q)
        );
        is(iri.fragment(), cases[i].f,
            std::string(cases[i].i) + " ("
            + std::string(cases[i].b) + ") FRAGMENT = "
            + std::string(cases[i].f)
        );
    }
    done_testing();
});

#endif
