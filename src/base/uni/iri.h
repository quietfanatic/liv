// A library for IRIs (Internationalized Resource Identifiers).
// Basically URIs but unicode.
// Under heavy development!  Don't use for anything important.
//
// Requires C++17 or later.
//
///// IRI HANDLING, POSSIBLE DEVIATIONS FROM SPECIFICATIONS
//
// This library is scheme-agnostic.  Parsing is the same for all schemes, so if
// there's a scheme that uses non-standard syntax it may not work properly.
//
// The authority (hostname or IP address, port, possible username) is opaque to
// this library.  It might let through some invalid authority components.
//
// Unlike most URI parsing libraries, this will leave non-ascii UTF-8 as-is,
// without %-encoding it, which is what makes them IRIs.  This library does not
// validate UTF-8 sequences.  If invalid UTF-8 is given, it will be passed
// through.
//
// Uppercase ASCII in the scheme and authority will be canonicalized to
// lowercase.  Non-ASCII is NOT canonicalized to lowercase in the authority
// (and it's forbidden in the scheme).
//
// ASCII Whitespace is rejected as invalid in all cases.  This may differ from
// other URI libraries, which may accept whitespace for some schemes such as
// data:.  Non-ASCII whitespace is passed through, since detecting it would
// require importing unicode tables, which are very large.
//
// IRIs with a path that starts with /.. will be rejected, unlike with most URI
// libraries, which will silently drop the .. segment.
//
// IRIs in this library cannot be longer than 65535 bytes.
//
// Since this is a very new and undertested library, there are likely to be some
// errors in handling IRIs.  If the behavior differs from the specifications:
//     https://datatracker.ietf.org/doc/html/rfc3987 - IRI
//     https://datatracker.ietf.org/doc/html/rfc3986 - URI
// then it is this library that is incorrect.
//
///// Interface
//
// None of the strings returned by any methods will be NUL-terminated.
//
// Will not throw when given an invalid IRI spec.  Instead will mark the IRI as
// invalid, and all accessors will return false or empty.  You can see what went
// wrong by looking at the return of possibly_invalid_spec().
//
// The component getter functions will not decode % sequences, because which
// characters have to be % encoding can be application-specific.  Call decode()
// yourself on the results when you want to decode them.
//
// This IRI class is pretty lightweight, with one reference-counted string and
// four uint16s.  16 bytes on 32-bit and 24 bytes on 64-bit.

#pragma once

#include "common.h"
#include "strings.h"

namespace uni {
inline namespace iri {

constexpr uint32 maximum_length = uint16(-1);

 // Replace reserved characters with % sequences
UniqueString encode (Str);
 // Replace % sequences with their characters.  If there's an invalid escape
 // sequence, leaves it as is.
UniqueString decode (Str);

 // The first component that the given IRI reference has
enum IRIRelativity : uint8 {
    SCHEME,       // scheme://auth/path?query#fragment
    AUTHORITY,    // //auth/path?query#fragment
    PATHABSOLUTE, // /path?query#fragment
    PATHRELATIVE, // path?query#fragment
    QUERY,        // ?query#fragment
    FRAGMENT,     // #fragment
    N_CLASSES
};

 // Return what kind of relative reference this is.  This only does basic
 // detection, and when given an invalid reference, may return anything.  To be
 // sure that the reference is valid, resolve it into a full IRI.
IRIRelativity classify_reference (Str);

struct IRI {
     // Construct the empty IRI.  This is not a valid IRI.
    IRI () { }
     // Construct from an IRI string.  Does validation and canonicalization.  If
     // base is provided, resolved ref as a IRI reference (AKA a relative IRI)
     // with base as its base. If base is not provided, ref must be an absolute
     // IRI with scheme included.
    explicit IRI (Str ref, const IRI& base = IRI());
     // Construct an already-parsed IRI.  This will not do any validation.  If
     // you provide invalid parameters, you will wreak havoc and mayhem.
    IRI (
        AnyString&& spec,
        uint16 colon_position, uint16 path_position,
        uint16 question_position, uint16 hash_position
    );

     // Copy and move construction and assignment
    IRI (const IRI& o);
    IRI (IRI&& o);
    IRI& operator = (const IRI& o);
    IRI& operator = (IRI&& o);

     // Returns whether this IRI is valid or not.  If the IRI is invalid, all
     // bool accessors will return false and all OldStr and IRI accessors will
     // return empty.
    bool is_valid () const;
     // Returns whether this IRI is empty.  The empty IRI is also invalid.
    bool is_empty () const;
     // Equivalent to is_valid
    explicit operator bool () const;

     // Gets the full text of the IRI only if this IRI is valid.
    const AnyString& spec () const;
     // Get full text of IRI even it is not valid.  This is only for diagnosing
     // what is wrong with the IRI.  Don't use it for anything important.
    const AnyString& possibly_invalid_spec () const;

     // Steal the spec string, leaving this IRI empty.
    AnyString move_spec ();
     // Steal the spec string even if it's invalid.
    AnyString move_possibly_invalid_spec ();

     // Returns an IRI reference that's relative to base, or just spec() if
     // this IRI has nothing in common with base.  Returning relative paths is
     // not yet implemented, so if this IRI and base differ in their paths, an
     // absolute path starting with / will be returned.
    UniqueString spec_relative_to (const IRI& base) const;

     // Check for existence of components.
    bool has_scheme () const;
    bool has_authority () const;
    bool has_path () const;
    bool has_query () const;
    bool has_fragment () const;

     // If there is a path and the path starts with /
    bool is_hierarchical () const;

     // Get the scheme of the IRI.  Doesn't include the :.
     // This will always return something for a valid IRI.
    Str scheme () const;
     // Get the authority (host and port).  Doesn't include the //.  Will
     // return empty if has_authority is false.  May still return empty if
     // has_authority is true, but the IRI has an empty authority (e.g.
     // file:///foo/bar)
    Str authority () const;
     // Get the path component of the IRI.
     //   scheme://host/path => /path
     //   scheme://host/ => /
     //   scheme://host => (empty, has_path will be false)
     //   scheme:///path => /path
     //   scheme:/path => /path
     //   scheme:path => path
     // If has_path is true, will always return non-empty.
    Str path () const;
     // Get the query.  Will not include the ?.  May be existent but empty.
    Str query () const;
     // Get the fragment.  Will not include the #.  May be existent but empty.
    Str fragment () const;

     // Returns a new IRI with just the scheme (and the colon).
    IRI iri_with_scheme () const;
     // Get the origin (scheme plus authority if it exists).  Never ends with
     // a /.
    IRI iri_with_origin () const;
     // Get everything up to and including the last / in the path.  If this is
     // not a hierarchical scheme (path doesn't start with /), returns empty.
    IRI iri_without_filename () const;
     // Get the scheme, authority, and path but not the query or fragment.
    IRI iri_without_query () const;
     // Get everything but the fragment
    IRI iri_without_fragment () const;

     // The following are the same as above, but return a raw Str instead of a
     // new IRI.  This saves a string copy, but can cost an extra parse if you
     // turn the Str back into an IRI.
    Str spec_with_scheme () const;
    Str spec_with_origin () const;
    Str spec_without_filename () const;
    Str spec_without_query () const;
    Str spec_without_fragment () const;

    Str path_without_filename () const;

     // Destruct this object
    ~IRI ();

     // Comparisons just do string comparisons on the spec
#define IRI_FRIEND_OP(op) \
    friend auto operator op (const IRI& a, const IRI& b) { \
        return a.spec_ op b.spec_; \
    }
#if __cplusplus >= 202002L
    IRI_FRIEND_OP(<=>)
#else
    IRI_FRIEND_OP(<)
    IRI_FRIEND_OP(<=)
    IRI_FRIEND_OP(==)
    IRI_FRIEND_OP(!=)
    IRI_FRIEND_OP(>=)
    IRI_FRIEND_OP(>)
#endif
#undef IRI_FRIEND_OP

  private:
     // Full text of the IRI
    AnyString spec_;
     // All the following markers are 0 for an invalid IRI.
     // Offset of the : after the scheme.
    uint16 colon_ = 0;
     // Offset of the start of the path.  path_ > colon_.
    uint16 path_ = 0;
     // Offset of the ? for the query, or the end of the path if there is no ?.
     // question_ >= path_.
    uint16 question_ = 0;
     // Offset of the # for the fragment, or the end of the query (or path) if
     // there is no #.  hash_ >= question_.
    uint16 hash_ = 0;
};

} // inline namespace iri
} // namespace uni

// I was going to specialize std::hash, but using IRIs as keys in an
// unordered_map would likely be a mistake, since you can just use Strings or
// Strs instead with the same behavior but less weight.
