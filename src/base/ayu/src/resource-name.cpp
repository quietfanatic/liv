#include "../resource-name.h"

#include <cassert>
#include <vector>
#include "../describe.h"
#include "char-cases-private.h"
#include "resource-private.h"

using namespace std::literals;

namespace ayu {

///// RESOURCE NAMES

String canonicalize (Str name) {
     // TODO: make empty resource name invalid
    if (name == "") throw X::InvalidResourceName(String(name));
    if (name == "#") return String(name);
     // Validate, Split and normalize
    std::vector<Str> segments;
    usize segment_start = 0;
    for (usize i = 0; i <= name.size(); i++)
    switch (i == name.size() ? '/' : name[i]) {
        case '#':
            throw X::GenericError("Fragments in resource names are NYI");
        case '"': case '*': case ':': case '<':
        case '>': case '?': case '\\': case '|':
            throw X::InvalidResourceName(String(name));
        case '/': {
            Str segment = Str(&name[segment_start], i - segment_start);
            if (segment == "") {
                if (i == 0 || i == name.size()) {
                    segments.push_back(segment);
                }
                else {
                     // Multiple /s in a row, do nothing
                }
            }
            else if (segment == "."sv) {
                 // Ignore /./
            }
            else if (segment == ".."sv) {
                if (segments.size() == 0) {
                     // Let ..s accumulate at the front
                    segments.push_back(segment);
                }
                else if (segments.back() == "") {
                     // Can't back up past root
                    throw X::ResourceNameOutsideRoot(String(name));
                }
                else {
                     // Cancel one segment and one ..
                    segments.pop_back();
                }
            }
            else {
                 // Ordinary segment
                segments.push_back(segment);
            }
            segment_start = i + 1;
            break;
        }
        default: { }
    }
    assert(segments.size());
     // Rejoin
    String r = String(segments[0]);
    for (usize i = 1; i < segments.size(); i++) {
        r = r + '/' + segments[i];
    }
    return r;
}

bool is_absolute (Str name) {
     // TODO: scheme
    return !name.empty() && name[0] == '/';
}

 // Kinda wasteful since we're calling canonicalize a lot, but it will hardly
 // matter in the long run.
String resolve (Str name, Str base) {
    String canon_name = canonicalize(name);
    if (is_absolute(canon_name)) return canon_name;
     // First resolve base
    String canon_base;
    if (base == "") {
        if (auto res = current_resource()) {
            canon_base = res.name();
        }
        else {
            throw X::UnresolvedResourceName(std::move(canon_name));
        }
    }
    else {
        canon_base = canonicalize(base);
        if (is_relative(canon_base)) {
            canon_base = resolve(canon_base);
        }
    }
    assert(is_absolute(canon_base));

     // # always refers to current file
    if (canon_name == "#") return canon_base;

    usize last_slash = canon_base.rfind('/');
    assert(last_slash != String::npos);
    canon_base.resize(last_slash + 1);
    String r = canonicalize(canon_base + canon_name);
    assert(is_absolute(r));
    return r;
}

///// RESOURCE SCHEMES

void ResourceScheme::activate () const {
    auto& schemes = universe().schemes;
     // Validate
    if (scheme_name.size() == 0) {
        throw X::InvalidResourceScheme(String(scheme_name));
    }
    for (const char& c : scheme_name)
    switch (c) {
        case ANY_LOWERCASE: break;
        case ANY_UPPERCASE:
        case ANY_DECIMAL_DIGIT:
        case '+': case '-': case '.':
            if (&c == &scheme_name.front()) {
                throw X::InvalidResourceScheme(String(scheme_name));
            }
            else break;
    }
     // Register
    auto [iter, emplaced] = schemes.emplace(scheme_name, this);
    if (!emplaced) throw X::DuplicateResourceScheme(String(scheme_name));
}
void ResourceScheme::deactivate () const {
    auto& schemes = universe().schemes;
    schemes.erase(scheme_name);
}

} // namespace ayu

///// AYU DESCRIPTIONS

AYU_DESCRIBE(ayu::X::InvalidResourceName,
    delegate(base<ayu::X::Error>()),
    elems(elem(&X::InvalidResourceName::name))
)
AYU_DESCRIBE(ayu::X::UnresolvedResourceName,
    delegate(base<ayu::X::Error>()),
    elems(elem(&X::UnresolvedResourceName::name))
)
AYU_DESCRIBE(ayu::X::ResourceNameOutsideRoot,
    delegate(base<ayu::X::Error>()),
    elems(elem(&X::ResourceNameOutsideRoot::name))
)
AYU_DESCRIBE(ayu::X::DuplicateResourceScheme,
    delegate(base<ayu::X::Error>()),
    elems(
        elem(&X::DuplicateResourceScheme::scheme)
    )
)

///// TESTS

#ifndef TAP_DISABLE_TESTS
#include "../../tap/tap.h"

static tap::TestSet tests ("base/ayu/resource-name", []{
    using namespace tap;
    auto tc = [](Str name, Str expected){
        is(canonicalize(name), expected,
            "canonicalize " + name + " => " + expected
        );
    };
    tc("/", "/");
    tc("foo", "foo");
    tc("/foo", "/foo");
    tc("foo/", "foo/");
    tc("foo/./bar", "foo/bar");
    tc("foo/../bar", "bar");
    tc("../bar", "../bar");
    tc("foo/../../bar", "../bar");
    throws<X::InvalidResourceName>([]{
        canonicalize("");
    }, "empty string is not valid resource name");
    throws<X::InvalidResourceName>([]{
        canonicalize("foo*bar");
    }, "* is not allowed in resource name");
    throws<X::ResourceNameOutsideRoot>([]{
        canonicalize("/..");
    }, "/.. is not allowed");
    throws<X::ResourceNameOutsideRoot>([]{
        canonicalize("/foo/../../bar");
    }, "/foo/../../bar is not allowed");
    auto tr = [](Str name, Str base, Str expected){
        is(resolve(name, base), expected,
            "resolve " + name + " " + base + " => " + expected
        );
    };
    tr("foo", "/", "/foo");
    tr("foo", "/bar", "/foo");
    tr("foo", "/bar/", "/bar/foo");
    tr("./foo", "/bar/", "/bar/foo");
    tr("../foo", "/bar/", "/foo");
    throws<X::ResourceNameOutsideRoot>([]{
        resolve("../foo", "/bar");
    }, "resolve ../foo /bar is an error");
    done_testing();
});
#endif
