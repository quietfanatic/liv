#pragma once

#ifdef LIV_PROFILE
#include "../dirt/uni/io.h"
#include "../dirt/uni/time.h"
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
struct RenderParams;
struct Settings;
struct SortMethod;

#ifdef LIV_PROFILE
inline void plog (Str s) {
    static double start = uni::now();
    warn_utf8(cat("[", uint64((uni::now() - start) * 1000000), "] ", s, "\n"));
}
#else
template <class C> void plog (C&&) { }
#endif

} // namespace liv
