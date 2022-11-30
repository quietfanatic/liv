#pragma once

#include <exception>
#include "../location.h"
#include "../resource.h"

namespace ayu::in {

enum LocationForm {
    ROOT,
    KEY,
    INDEX,
     // Internal for lazy error throwing
    ERROR_LOC,
};

 // A version of Locations that can be allocated on the stack for very cheap
struct TempLocation {
    uint8 form;
    TempLocation (uint8 f) : form(f) { }
};

struct RootTempLocation : TempLocation {
    Resource resource;
    RootTempLocation (Resource&& res) :
        TempLocation(ROOT), resource(res)
    { }
};
struct KeyTempLocation : TempLocation {
    TempLocation* parent;
    Str key;
    KeyTempLocation (TempLocation* p, Str k) : 
        TempLocation(KEY), parent(p), key(k)
    { }
};
struct IndexTempLocation : TempLocation {
    TempLocation* parent;
    usize index;
    IndexTempLocation (TempLocation* p, usize i) :
        TempLocation(INDEX), parent(p), index(i)
    { }
};

 // Transform temporary location to permanent location, probably for error
 // reporting.
Location make_permanent (TempLocation* t);

Location make_error_location (std::exception_ptr&& e);

}
