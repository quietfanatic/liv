#include "arrays.h"
#include "strings.h"

#include <unordered_map>
#include <vector>

#ifndef TAP_DISABLE_TESTS
#include "../tap/tap.h"

using namespace uni;

AnyArray<int> t (AnyArray<int>&& a) {
    return a;
}
AnyArray<int> t2 (const AnyArray<int>& a) {
    return a;
}

UniqueArray<int> t3 (int* dat, usize len) {
    return UniqueArray<int>(dat, len);
}

std::vector<int> c3 (int* dat, usize len) {
    return std::vector<int>(dat, dat+len);
}

UniqueArray<int> t4 (int* a, int* b) {
    return UniqueArray<int>(a, b);
}

UniqueArray<char> t5 (char* a, char* b) {
    return UniqueArray<char>(a, b);
}

AnyArray<int> t6 (const UniqueArray<int>& a) {
    return a;
}
AnyArray<int> t7 (const UniqueArray<int>& a) {
    return AnyArray<int>::Copy(a);
}

AnyArray<int> t8 (const std::vector<int>& v) {
    return UniqueArray<int>(v);
}

void t9 (AnyArray<int>& a, const AnyArray<int>& b) {
    a = b;
}
void c9 (std::vector<int>& a, const std::vector<int>& b) {
    a = b;
}

constexpr int foos [] = {2, 4, 6, 8, 10, 12};

AnyArray<int> t10 () {
    return AnyArray<int>(Slice<int>(foos));
}

AnyArray<char> t11 () {
    return "formidable";
}

AnyArray<std::pair<usize, usize>> t12 (const std::unordered_map<int, int>& m) {
    auto r = AnyArray<std::pair<usize, usize>>::Copy(m.begin(), m.end());
    return r;
}
AnyArray<std::pair<usize, usize>> t13 (const std::unordered_map<int, int>& m) {
    auto r = AnyArray<std::pair<usize, usize>>::Copy(m.begin(), m.size());
    return r;
}

void t14 (AnyArray<int>& v) {
    v.reserve(50);
}

void t15 (AnyArray<int>& v) {
    v.shrink_to_fit();
}

void t16 (AnyArray<int>& v) {
    v.make_unique();
}

void t17 (UniqueArray<int>& v) {
    v.resize(50);
}
void c17 (std::vector<int>& v) {
    v.resize(50);
}
void t18 (AnyArray<int>& v) {
    v.resize(50);
}

void t19 (AnyArray<int>& v) {
    v.push_back(99);
}
void c19 (std::vector<int>& v) {
    v.push_back(99);
}
void t20 (UniqueArray<int>& v) {
    v.push_back(99);
}
void t21 (AnyArray<int>& v) {
    v.pop_back();
}

AnyArray<int> t22 () {
    AnyArray<int> r;
    r.reserve(32);
    for (usize i = 0; i < 32; i++) {
        r.emplace_back(i);
    }
    return r;
}

AnyArray<int> b22 () {
    AnyArray<int> r;
    r.reserve(32);
    for (usize i = 0; i < 32; i++) {
        r.emplace_back_expect_capacity(i);
    }
    return r;
}

std::vector<int> c22 () {
    std::vector<int> r;
    r.reserve(32);
    for (usize i = 0; i < 32; i++) {
        r.emplace_back(i);
    }
    return r;
}

UniqueArray<int> t23 () {
    UniqueArray<int> r;
    r.reserve(32);
    for (usize i = 0; i < 32; i++) {
        r.emplace_back(i);
    }
    return r;
}

UniqueArray<int> b23 () {
    UniqueArray<int> r;
    r.reserve(32);
    for (usize i = 0; i < 32; i++) {
        r.emplace_back_expect_capacity(i);
    }
    return r;
}

void t24 (UniqueArray<int>& a) {
    a.emplace(32, 100);
}
void c24 (std::vector<int>& a) {
    a.emplace(a.begin() + 32, 100);
}

void t25 (AnyArray<int>& a) {
    a.erase(44, 2);
}

const char* t26 (AnyArray<char>& a) {
    return a.c_str();
}

std::string c29 (std::string&& s) {
    return std::move(s) + "foo" + "bar";
}

UniqueString t29 (UniqueString&& s) {
    return cat(std::move(s), "foo", "bar");
}
UniqueString b29 (const UniqueString& s) {
    return cat(s, "foo", "bar");
}

UniqueString t28 (UniqueString&& s) {
    return cat(std::move(s), "foo"_s, "bar"_s);
}

UniqueString t27 (const char* a, const char* b) {
    return cat(a, b);
}

UniqueString t30 () {
    return cat("foo"_s, 4, "bar"_s);
}

UniqueString t31 () {
    return cat("foo"_s, 5.0, "bar"_s);
}

UniqueString t32 (double d) {
    return cat("foo"_s, d, "bar"_s);
}

NOINLINE
void t33a (AnyString&& a) {
    printf("%s\n", a.c_str());
}
NOINLINE
void t33b (AnyString a) {
    t33a(std::move(a));
}
void t33c (AnyString&& a) {
    t33b(std::move(a));
}
NOINLINE
void c33a (std::string&& a) {
    printf("%s\n", a.c_str());
}
NOINLINE
void c33b (std::string a) {
    c33a(std::move(a));
}
void c33c (std::string&& a) {
    c33b(std::move(a));
}

static tap::TestSet tests ("base/uni/arrays", []{
    using namespace tap;
    AnyArray<int> a;
    is(a.size(), usize(0), "empty array has size 0");
    is(a.data(), null, "empty-constructed array has null data");
    AnyArray<int> b = std::move(a);
    is(b.size(), usize(0), "move empty array");
    is(b.data(), null);
    AnyArray<int> c = b;
    is(c.size(), usize(0), "copy empty array");
    is(c.data(), null);

    c.push_back(4);
    is(c.size(), usize(1), "push_back");
    is(c[0], 4);
    for (usize i = 0; i < 50; i++) {
        c.push_back(i);
    }
    is(c.size(), usize(51));
    is(c[50], 49);

    is(c.unique(), true, "unique");
    AnyArray<AnyArray<int>> d (5, c);
    is(d.size(), usize(5), "array with non-trivial type");
    is(c.unique(), false, "AnyArray buffer is not copied when AnyArray is copied");
    c.erase(1, 5);
    is(c.unique(), true, "copy on write");
    is(c.size(), usize(46), "erase");
    is(c[1], 5);
    is(d[0][1], 0, "other arrays sharing buffer are not changed");
    is(cat("foo"_s, 6, "bar"_s), "foo6bar"_s, "cat()");

    done_testing();
});

#endif
