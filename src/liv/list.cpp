#include "list.h"

#include "../dirt/iri/path.h"
#include "../dirt/uni/io.h"

namespace liv {

UniqueArray<IRI> read_list (const IRI& loc) {
    UniqueArray<IRI> r;

    UniqueString line;
    if (loc == "liv:stdin") {
         // TODO: Make string_from_file support filehandles
        for (;;) {
            int c = getchar();
            if (c == EOF) break;
            else if (c == '\n') {
                if (line) {
                    r.emplace_back(iri::from_fs_path(line));
                    line = "";
                }
            }
            else line.push_back(c);
        }
    }
    else {
        for (char c : string_from_file(iri::to_fs_path(loc))) {
            if (c == '\n') {
                if (line) {
                    r.emplace_back(iri::from_fs_path(line, loc));
                    line = "";
                }
            }
            else line.push_back(c);
        }
    }
    return r;
}

void write_list (const IRI& loc, Slice<IRI> entries) {
    UniqueString s;
    for (auto& e : entries) {
        encat(s, iri::to_fs_path(e), '\n');
    }
    string_to_file(s, iri::to_fs_path(loc));
}

} // liv
