#pragma once

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

} // namespace liv
