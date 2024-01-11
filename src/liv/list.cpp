#include "list.h"

#include "../dirt/iri/path.h"
#include "../dirt/uni/io.h"
#include "sort.h"

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
            else if (c == '\r') {
                 // Shouldn't get \r\n with stdin, but for consistency's sake
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
            else if (c == '\r') { }
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

void add_to_list (const IRI& list, const IRI& entry, SortMethod sort) {
     // Read
    UniqueArray<IRI> entries;
    try {
        entries = read_list(list);
    }
    catch (Error& e) {
        if (e.code == e_OpenFailed) {
             // New file, create it implicitly
        }
        else throw;
    }
     // Add
    entries.push_back(entry);
     // Sort and remove duplicates
    if (sort.criterion != SortCriterion::Unsorted) {
        sort_iris(entries.begin(), entries.end(), sort);
        auto new_end = std::unique(entries.begin(), entries.end());
        entries.impl.size = new_end - entries.impl.data;
    }
     // Write
    write_list(list, entries);
}

void remove_from_list (const IRI& list, const IRI& entry) {
    auto entries = read_list(list);
    auto new_end = std::remove(entries.begin(), entries.end(), entry);
    entries.impl.size = new_end - entries.impl.data;
    write_list(list, entries);
}

} // liv
