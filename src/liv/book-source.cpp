#include "book-source.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/iri/path.h"
#include "../dirt/uni/errors.h"
#include "settings.h"

namespace liv {

static void validate_location (const IRI& loc) {
    if (loc.scheme() != "file" || !loc.hierarchical()) {
        raise(e_General, "IRI given to BookSource is not a proper file IRI");
    }
}

void BookSource::validate () const {
    switch (type) {
        case BookType::Misc: {
            for (auto& arg : locations) validate_location(arg);
            break;
        }
        case BookType::Folder: {
            if (locations.size() != 1) {
                raise(e_General, "BookType::Folder cannot have multiple locations");
            }
            validate_location(locations[0]);
            if (locations[0].path().back() != '/') {
                raise(e_General, "Location for BookType::Folder must end with /");
            }
            break;
        }
        case BookType::FileWithNeighbors: {
            if (locations.size() != 1) {
                raise(e_General, "BookType::FileWithNeighbors cannot have multiple locations");
            }
            validate_location(locations[0]);
            if (locations[0].path().back() == '/') {
                raise(e_General, "Location for BookType::FileWithNeighbors must not end with /");
            }
            break;
        }
        case BookType::List: {
            if (locations.size() != 1) {
                raise(e_General, "BookType::List cannot have multiple locations");
            }
            validate_location(locations[0]);
            if (locations[0].path().back() == '/') {
                raise(e_General, "Location for BookType::List must not end with /");
            }
            break;
        }
        default: never();
    }
}

const IRI& BookSource::location_for_memory () const {
    static constexpr IRI empty;
    switch (type) {
        case BookType::Misc: return empty;
        case BookType::Folder: return locations[0];
        case BookType::List: {
            if (locations[0] == "liv:stdin") return empty;
            else return locations[0];
        }
        case BookType::FileWithNeighbors: return empty;
        default: never();
    }
}

const IRI& BookSource::base_for_page_rel_book () const {
    switch (type) {
        case BookType::Misc: return iri::working_directory();
        case BookType::Folder: return locations[0];
        case BookType::List: {
            if (locations[0] == "liv:stdin") return iri::working_directory();
            else return locations[0];
        }
        case BookType::FileWithNeighbors: return locations[0];
        default: never();
    }
}

IRI BookSource::base_for_page_rel_book_parent () const {
    const IRI* r;
    switch (type) {
        case BookType::Misc: {
            r = &iri::working_directory();
            break;
        }
        case BookType::Folder: {
            expect(locations[0].path().back() == '/');
            expect(locations[0] && locations[0].hierarchical());
            if (auto parent = locations[0].chop_last_slash()) {
                return parent;
            }
            else {
                expect(parent.error() == iri::Error::PathOutsideRoot);
                r = &locations[0];
                break;
            }
        }
        case BookType::List: {
            if (locations[0] == "liv:stdin") r = &iri::working_directory();
            else r = &locations[0];
            break;
        }
        case BookType::FileWithNeighbors: {
            expect(locations[0].path().back() != '/');
            r = &locations[0];
            break;
        }
        default: never();
    }
    return *r;
}

} using namespace liv;

AYU_DESCRIBE(liv::BookType,
    values(
        value("args", BookType::Misc),
        value("folder", BookType::Folder),
        value("list", BookType::List),
        value("file_with_neighbors", BookType::FileWithNeighbors)
    )
)

AYU_DESCRIBE(liv::BookSource,
    elems(
        elem(&BookSource::type),
        elem(&BookSource::locations)
    )
)
