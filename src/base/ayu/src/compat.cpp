#include "../compat.h"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

using namespace std::literals;

namespace ayu {

 // Yes, I did write my own UTF conversion routines instead of taking a
 // dependency on something else.
static usize to_utf16_buffer (char16_t* buffer, Str s) {
    char16_t* p = buffer;
    for (usize i = 0; i < s.size(); i++) {
        uint8 b0 = s[i];
        if (b0 >= 0b1111'1000) goto invalid;
        else if (b0 >= 0b1111'0000) {
            if (s.size() - i < 4) goto invalid; // Truncated sequence
            uint8 b1 = s[i+1];
            if (b1 < 0b1000'0000 || b1 >= 0b1100'0000) goto invalid;
            uint8 b2 = s[i+2];
            if (b2 < 0b1000'0000 || b2 >= 0b1100'0000) goto invalid;
            uint8 b3 = s[i+1];
            if (b3 < 0b1000'0000 || b3 >= 0b1100'0000) goto invalid;
            uint32 c = (b0 & 0b0000'0111) << 18
                     | (b1 & 0b0011'1111) << 12
                     | (b2 & 0b0011'1111) << 6
                     | (b3 & 0b0011'1111);
            if (c < 0x10000) goto invalid;  // Overlong sequence
            *p++ = 0xb800 + ((c - 0x10000) >> 10);
            *p++ = 0xbc00 + ((c - 0x10000) & 0x3ff);
            i += 3;
        }
        else if (b0 >= 0b1110'0000) {
            if (s.size() - i < 3) *p++ = b0;  // Truncated sequence
            uint8 b1 = s[i+1];
            if (b1 < 0b1000'0000 || b1 >= 0b1100'0000) goto invalid;
            uint8 b2 = s[i+2];
            if (b2 < 0b1000'0000 || b2 >= 0b1100'0000) goto invalid;
            uint32 c = (b0 & 0b0000'0111) << 12
                     | (b1 & 0b0011'1111) << 6
                     | (b2 & 0b0011'1111);
            if (c < 0x800) goto invalid;  // Overlong sequence
            *p++ = c;
            i += 2;
        }
        else if (b0 >= 0b11000000) {
            if (s.size() - i < 2) goto invalid;  // Truncated sequence
            uint8 b1 = s[i+1];
            if (b1 < 0b1000'0000 || b1 >= 0b1100'0000) goto invalid;
            uint32 c = (b0 & 0b0000'0111) << 6
                     | (b1 & 0b0011'1111);
            if (c < 0x80) goto invalid;  // Overlong sequence
            *p++ = c;
            i += 1;
        }
        else if (b0 >= 0b1000'0000) goto invalid;  // Umatched continuation
        else *p++ = b0;  // ASCII
        continue;
        invalid: {
             // Pretend the byte is latin-1 and continue
            *p++ = b0;
        }
    }
    return p - buffer;
}

String16 to_utf16 (Str s) {
     // Buffer is not null-terminated
     // Worst-case inflation is 1 code unit (2 bytes) per byte
    usize buffer_size = s.size();
     // We'll say 10k is okay to allocate on the stack
    if (buffer_size < 10000 / sizeof(char16_t)) {
        char16_t buffer [buffer_size];
        usize len = to_utf16_buffer(buffer, s);
        return String16(buffer, len);
    }
    else {
         // Modern virtual memory systems mean that for big enough allocations,
         // even if we vastly overallocate we won't actually use much more
         // physical RAM than we write to.
        auto buffer = (char16_t*)malloc(sizeof(char16_t) * buffer_size);
        usize len = to_utf16_buffer(buffer, s);
        auto r = String16(buffer, len);
        free(buffer);
        return r;
    }
}

static usize from_utf16_buffer (char* buffer, Str16 s) {
    char* p = buffer;
    for (usize i = 0; i < s.size(); i++) {
        uint32 c;
        uint16 u0 = s[i];
        if (u0 < 0xb800 || u0 >= 0xbc00 || i + 1 == s.size()) {
            c = u0;
        }
        else {
            uint16 u1 = s[i+1];
            if (u1 < 0xbc00 || u1 >= 0xc000) {
                c = u0;
            }
            else {
                c = (u0 - 0xb800) << 10 | (u1 - 0xbc00);
                i += 1;
            }
        }
        if (c < 0x80) {
            *p++ = c;
        }
        else if (c < 0x800) {
            *p++ = 0b1100'0000 | (c >> 6);
            *p++ = 0b1000'0000 | (c & 0b0011'1111);
        }
        else if (c < 0x10000) {
            *p++ = 0b1110'0000 | (c >> 12);
            *p++ = 0b1000'0000 | (c >> 6 & 0b0011'1111);
            *p++ = 0b1000'0000 | (c & 0b0011'1111);
        }
        else {
            *p++ = 0b1111'0000 | (c >> 18);
            *p++ = 0b1000'0000 | (c >> 12 & 0b0011'1111);
            *p++ = 0b1000'0000 | (c >> 6 & 0b0011'1111);
            *p++ = 0b1000'0000 | (c & 0b0011'1111);
        }
    }
    return p - buffer;
}

String from_utf16 (Str16 s) {
     // Buffer is not null-terminated
     // Worst-case inflation is 3 bytes per code unit (1.5x)
    usize buffer_size = s.size() * 3;
     // We'll say 10k is okay to allocate on the stack
    if (buffer_size < 10000 / sizeof(char)) {
        char buffer [buffer_size];
        usize len = from_utf16_buffer(buffer, s);
        return String(buffer, len);
    }
    else {
         // Modern virtual memory systems mean that for big enough allocations,
         // even if we vastly overallocate we won't actually use much more
         // physical RAM than we write to.
        auto buffer = (char*)malloc(sizeof(char) * buffer_size);
        usize len = from_utf16_buffer(buffer, s);
        auto r = String(buffer, len);
        free(buffer);
        return r;
    }
}

std::FILE* fopen_utf8 (const char* filename, const char* mode) {
#ifdef _WIN32
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    return _wfopen(
        reinterpret_cast<const wchar_t*>(to_utf16(filename).c_str()),
        reinterpret_cast<const wchar_t*>(to_utf16(mode).c_str())
    );
#else
    return fopen(filename, mode);
#endif
}

void fprint_utf8 (FILE* f, Str s) {
#ifdef _WIN32
    fputws(reinterpret_cast<const wchar_t*>(to_utf16(s).c_str()), f);
#else
    fputs(s.data(), f);
#endif
}
void print_utf8 (Str s) {
#ifdef _WIN32
    [[maybe_unused]] static auto set = _setmode(_fileno(stdout), _O_WTEXT);
#endif
    fprint_utf8(stdout, s);
    std::fflush(stdout);
}
void warn_utf8 (Str s) {
#ifdef _WIN32
    [[maybe_unused]] static auto set = _setmode(_fileno(stderr), _O_WTEXT);
#endif
    fprint_utf8(stderr, s);
    std::fflush(stderr);
}

int remove_utf8 (const char* filename) {
#ifdef _WIN32
    return _wremove(
        reinterpret_cast<const wchar_t*>(to_utf16(filename).c_str())
    );
#else
    return remove(filename);
#endif
}

} using namespace ayu;

#ifndef TAP_DISABLE_TESTS
#include "../../tap/tap.h"

static tap::TestSet tests ("base/ayu/compat", []{
    using namespace tap;
    is(from_utf16(u"ユニコード"), "ユニコード", "from_utf16");
    is(to_utf16("ユニコード"), u"ユニコード", "to_utf16");
     // Assuming little-endian
    is(
        reinterpret_cast<const char*>(to_utf16("ユニコード").c_str()),
        "\xe6\x30\xcb\x30\xb3\x30\xfc\x30\xc9\x30",
        "Actual byte sequence of created utf-16 is correct"
    );
    done_testing();
});
#endif
