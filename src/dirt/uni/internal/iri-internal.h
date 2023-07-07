#pragma once

// Inlined

namespace uni {
inline namespace iri {

constexpr IRI::IRI (AnyString&& spec, uint16 c, uint16 p, uint16 q, uint16 h) :
    spec_(move(spec)), colon_(c), path_(p), question_(q), hash_(h)
{ }

constexpr IRI::IRI (const IRI& o) = default;
constexpr IRI::IRI (IRI&& o) :
    spec_(move(o.spec_)),
    colon_(o.colon_),
    path_(o.path_),
    question_(o.question_),
    hash_(o.hash_)
{ o.colon_ = o.path_ = o.question_ = o.hash_ = 0; }
constexpr IRI& IRI::operator = (const IRI& o) {
    if (this == &o) return *this;;
    this->~IRI();
    new (this) IRI(o);
    return *this;
}
constexpr IRI& IRI::operator = (IRI&& o) {
    if (this == &o) return *this;
    this->~IRI();
    new (this) IRI(move(o));
    return *this;
}

constexpr bool IRI::is_valid () const { return colon_; }
constexpr bool IRI::is_empty () const { return spec_.empty(); }
constexpr IRI::operator bool () const { return colon_; }

static constexpr const AnyString empty = StaticString();

constexpr const AnyString& IRI::spec () const {
    if (colon_) return spec_;
    else return empty;
}
constexpr const AnyString& IRI::possibly_invalid_spec () const {
    return spec_;
}

constexpr AnyString IRI::move_spec () {
    if (!colon_) return empty;
    AnyString r = move(spec_);
    *this = IRI();
    return r;
}
constexpr AnyString IRI::move_possibly_invalid_spec () {
    AnyString r = move(spec_);
    *this = IRI();
    return r;
}

constexpr bool IRI::has_scheme () const { return colon_; }
constexpr bool IRI::has_authority () const { return path_ >= colon_ + 3; }
constexpr bool IRI::has_path () const { return question_ > path_; }
constexpr bool IRI::has_query () const { return hash_ > question_; }
constexpr bool IRI::has_fragment () const { return hash_ && spec_.size() > hash_; }

constexpr bool IRI::is_hierarchical () const {
    return has_path() && spec_[path_] == '/';
}

constexpr Str IRI::scheme () const {
    if (!has_scheme()) return "";
    return spec_.slice(0, colon_);
}
constexpr Str IRI::authority () const {
    if (!has_authority()) return "";
    return spec_.slice(colon_ + 3, path_);
}
constexpr Str IRI::path () const {
    if (!has_path()) return "";
    return spec_.slice(path_, question_);
}
constexpr Str IRI::query () const {
    if (!has_query()) return "";
    return spec_.slice(question_ + 1, hash_);
}
constexpr Str IRI::fragment () const {
    if (!has_fragment()) return "";
    return spec_.slice(hash_ + 1, spec_.size());
}

constexpr IRI IRI::iri_with_scheme () const {
    if (!has_scheme()) return IRI();
    return IRI(
        spec_.slice(0, colon_+1),
        colon_, colon_+1, colon_+1, colon_+1
    );
}
constexpr IRI IRI::iri_with_origin () const {
    if (!has_scheme()) return IRI();
    return IRI(
        spec_with_origin(),
        colon_, path_, path_, path_
    );
}
constexpr IRI IRI::iri_without_filename () const {
    if (!is_hierarchical()) return IRI();
    uint32 i = question_;
    while (spec_[i] != '/') i--;
    return IRI(
        spec_.slice(0, i+1),
        colon_, path_, i+1, i+1
    );
}
constexpr IRI IRI::iri_without_query () const {
    if (!has_scheme()) return IRI();
    return IRI(
        spec_.slice(0, question_),
        colon_, path_, question_, question_
    );
}
constexpr IRI IRI::iri_without_fragment () const {
    if (!has_scheme()) return IRI();
    return IRI(
        spec_.slice(0, hash_),
        colon_, path_, question_, hash_
    );
}

constexpr Str IRI::spec_with_scheme () const {
    if (!has_scheme()) return "";
    return spec_.slice(0, colon_ + 1);
}
constexpr Str IRI::spec_with_origin () const {
    return has_authority()
        ? spec_.slice(0, path_)
        : colon_
            ? spec_.slice(0, colon_ + 1)
            : "";
}
constexpr Str IRI::spec_without_filename () const {
    if (is_hierarchical()) {
        uint32 i = question_;
        while (spec_[i-1] != '/') --i;
        return spec_.slice(0, i);
    }
    else {
        return spec_.slice(0, question_);
    }
}
constexpr Str IRI::spec_without_query () const {
    if (!has_scheme()) return "";
    return spec_.slice(0, question_);
}
constexpr Str IRI::spec_without_fragment () const {
    if (!has_scheme()) return "";
    return spec_.slice(0, hash_);
}

constexpr Str IRI::path_without_filename () const {
    if (is_hierarchical()) {
        uint32 i = question_;
        while (spec_[i-1] != '/') --i;
        return spec_.slice(path_, i);
    }
    else {
        return path();
    }
}

constexpr IRI::~IRI () { }

} // iri
} // uni
