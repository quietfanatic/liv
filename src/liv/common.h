#pragma once

#ifdef LIV_PROFILE
#include <cstdio>
#include <ctime> // Will be using POSIX functions though
#include "../dirt/uni/arrays.h"
#endif

namespace std::filesystem { }
namespace geo { }
namespace iri { struct IRI; }
using iri::IRI;
namespace uni { }

namespace liv {

using namespace geo;
using namespace uni;
namespace fs = std::filesystem;
struct App;
struct Book;
struct BookSource;
struct BookState;
struct BookView;
struct FormatList;
struct FormatToken;
struct Layout;
struct LayoutParams;
struct MemoryOfBook;
struct Page;
struct PageBlock;
struct Settings;
struct SortMethod;

#ifdef LIV_PROFILE
inline void plog (const char* s) {
    struct timespec t;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
    fprintf(stderr, "[%ld.%09ld] %s\n", long(t.tv_sec), long(t.tv_nsec), s);
}
#else
template <class C> void plog (C&&) { }
#endif

} // namespace liv
