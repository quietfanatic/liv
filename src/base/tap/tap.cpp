#include "tap.h"

#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif

#include <iostream>
#include <memory>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <typeinfo>
#include <vector>

namespace tap {
using namespace internal;
using namespace std::string_literals;

 ///// TestSet implementation

#ifndef TAP_DISABLE_TESTS

namespace internal {
    struct TestSetData {
        std::string name;
        void(* code )();
    };
}
static std::vector<std::unique_ptr<TestSetData>>& testers () {
    static std::vector<std::unique_ptr<TestSetData>> testers;
    return testers;
}

TestSet::TestSet (const std::string& name, void(* code )()) {
    testers().emplace_back(new TestSetData{name, code});
}

#endif

 ///// GLOBAL STATE

static unsigned num_planned = 0;
static unsigned num_tested = 0;
static unsigned num_to_todo = 0;
static bool block_todo = false;
static std::string todo_excuse;
static void(* print )(const std::string&) = [](const std::string& s){
    fputs(s.c_str(), stdout);
};

 ///// API

void plan (unsigned num_tests) {
    num_planned = num_tests;
    num_tested = 0;
    num_to_todo = 0;
    print("1.." + std::to_string(num_tests) + "\n");
}

void done_testing () {
    plan(num_tested);
}

bool ok (bool succeeded, const std::string& name) {
    num_tested += 1;
    std::string m;
    if (!succeeded) {
        m += "not ";
    }
    m += "ok " + std::to_string(num_tested);
    if (!name.empty()) {
        m += " " + name;
    };
    if (num_to_todo || block_todo) {
        m += " # TODO " + todo_excuse;
        if (num_to_todo) num_to_todo--;
    }
    print(m + "\n");
    return succeeded;
}
bool try_ok (CallbackRef<bool()> code, const std::string& name) {
    return fail_on_throw([&]{
        return ok(code(), name);
    }, name);
}

bool is_strcmp(const char* got, const char* expected, const std::string& name) {
    if (!got && !expected) {
        return pass(name);
    }
    else if (!got || !expected) {
        fail(name);
        diag_unexpected(got, expected);
        return false;
    }
    else if (0 == strcmp(got, expected)) {
        return pass(name);
    }
    else {
        fail(name);
        diag_unexpected(got, expected);
        return false;
    }
}
bool try_is_strcmp(CallbackRef<const char*()> code, const char* expected, const std::string& name) {
    return fail_on_throw([&]{
        return is_strcmp(code(), expected, name);
    }, name);
}
bool isnt_strcmp(const char* got, const char* unexpected, const std::string& name) {
    if (!got && !unexpected) return pass(name);
    else if (!got || !unexpected) return fail(name);
    else return ok(0 != strcmp(got, unexpected), name);
}
bool try_isnt_strcmp(CallbackRef<const char*()> code, const char* unexpected, const std::string& name) {
    return fail_on_throw([&]{
        return isnt_strcmp(code(), unexpected, name);
    }, name);
}

struct plusminus {
    double range;
    double center;
};
template <>
struct Show<plusminus> {
    std::string show (const plusminus& v) {
        return "within +/- " + Show<double>().show(v.range) + " of " + Show<double>().show(v.center);
    }
};

bool within (double got, double range, double expected, const std::string& name) {
    if (range < 0) range = -range;
    if (got >= expected - range && got <= expected + range) {
        return pass(name);
    }
    else {
        fail(name);
        plusminus pm = {range, expected};
        diag_unexpected(got, pm);
        return false;
    }
}
bool try_within (CallbackRef<double()> code, double range, double expected, const std::string& name) {
    return fail_on_throw([&]{
        return within(code(), range, expected, name);
    }, name);
}

bool doesnt_throw (CallbackRef<void()> code, const std::string& name) {
    return fail_on_throw([&]{
        code();
        return pass(name);
    }, name);
}

bool pass (const std::string& name) {
    return ok(true, name);
}
bool fail (const std::string& name) {
    return ok(false, name);
}

void todo (unsigned num, const std::string& excuse) {
    num_to_todo = num;
    todo_excuse = excuse;
}
void todo (const std::string& excuse, CallbackRef<void()> code) {
    auto old_excuse = todo_excuse;
    auto old_block_todo = block_todo;
    todo_excuse = excuse;
    block_todo = true;
    code();
    todo_excuse = old_excuse;
    block_todo = old_block_todo;
}

void skip (unsigned num, const std::string& excuse) {
    for (unsigned int i = 0; i < num; i++) {
        num_tested++;
        print("ok " + std::to_string(num_tested) + " # SKIP " + excuse + "\n");
    }
}

void set_print (void(* f )(const std::string&)) {
    print = f;
}

void diag (const std::string& message) {
    print(" # " + message + "\n");
}

void BAIL_OUT (const std::string& reason) {
    printf("Bail out!  %s", reason.c_str());
    exit(1);
}

void allow_testing (int argc, char** argv, const char* test_flag) {
    tap::argc = argc;
    tap::argv = argv;
    if (test_flag && test_flag[0]) {
        if (argc >= 2 && 0==strcmp(argv[1], test_flag)) {
            if (argc >= 3) {
                run_test(argv[2]);
                exit(0);
            }
            else {
                list_tests();
                exit(0);
            }
        }
        return;  // escape here if no testing arguments.
    }
    else if (argc >= 2) {
        run_test(argv[1]);
        exit(0);
    }
    else {
        list_tests();
        exit(0);
    }
}

#ifndef TAP_DISABLE_TESTS
void run_test (const std::string& name) {
    for (auto& t : testers()) {
        if (t->name == name) {
            try {
                t->code();
            }
            catch (std::exception& e) {
                printf("Uncaught exception: %s\n", e.what());
                throw;
            }
            catch (...) {
                printf("Uncaught non-standard exception.");
                throw;
            }
            return;
        }
    }
    printf("1..1\nnot ok 1 - No test named %s has been compiled.\n", name.c_str());
}
#else
void run_test (const std::string&) {
    printf("1..0 # SKIP this program was compiled with testing disabled\n");
}
#endif

void list_tests () {
#ifndef TAP_DISABLE_TESTS
    for (auto& t : testers()) {
        print(t->name + "\n");
    }
#else
    print("(testing disabled)");
#endif
}

int argc = 0;
char** argv = nullptr;

///// Internal pipeworks

namespace internal {
    std::string type_name (const type_info& type) {
#if __has_include(<cxxabi.h>)
        int status;
        char* demangled = abi::__cxa_demangle(type.name(), nullptr, nullptr, &status);
        if (status != 0) return "(Failed to demangle "s + type.name() + ")";
        std::string r = const_cast<const char*>(demangled);
        free(demangled);
        return r;
#else
         // Probably MSVC, which automatically demangles.
        return std::string(type.name());
#endif
    }

    bool fail_on_throw(CallbackRef<bool()> code, const std::string& name) {
        try {
            return code();
        }
        catch (const scary_exception&) { throw; }
        catch (const std::exception& e) {
            fail(name);
            diag("Threw " + Show<std::exception>().show(e));
            return false;
        }
        catch (...) {
            fail(name);
            diag("Threw non-standard exception");
            return false;
        }
    }

    void diag_didnt_throw(const std::type_info& expected) {
        diag("Expected exception of type " + type_name(expected));
    }
    void diag_wrong_exception(const std::exception& got, const std::type_info& expected) {
        diag("Expected exception of type " + type_name(expected));
        diag("     Got " + Show<std::exception>().show(got));
    }
    void diag_wrong_exception_nonstandard(const std::type_info& expected) {
        diag("Expected exception of type " + type_name(expected));
        diag("     Got non-standard exception.");
    }

    std::string show_ptr (void* v) {
        if (v) {
            std::stringstream ss;
            ss << "0x" << v;
            return ss.str();
        }
        else {
            return "nullptr";
        }
    }
}

}  // namespace tap

///// Self tests

#ifdef TAP_SELF_TEST
#ifndef TAP_DISABLE_TESTS

static tap::TestSet self_tests ("base/tap/tap", []{
    using namespace tap;
    plan(51);
    diag(std::to_string(sizeof(std::string)));

    pass("pass passes");
    ok(true, "ok on true passes");
    try_ok([]{return true;}, "try_ok works");
    is((int)32, (int)32, "is on equal ints passes");
    try_is<int>([]{return 32;}, 32, "try_is works");
    is((float)32, (float)32, "is on equal floats passes");
    is((double)32, (double)32, "is on equal floats passes");
    is_strcmp("asdf", "asdf", "is_strcmp on equal strings passes");
    try_is_strcmp([]{return "asdf";}, "asdf", "try_is_strcmp works");
    is_strcmp((const char*)NULL, (const char*)NULL, "is_strcmp on NULLS passes");
    is("asdf", "asdf", "is on equal strings passes");
    is((const char*)NULL, (const char*)NULL, "is on const char* NULLS passes");
    is((int*)NULL, (int*)NULL, "is on int* NULLS passes");
    int heyguys = 9;
    is(&heyguys, &heyguys, "is can compare pointers");
    is(std::string("asdf"), std::string("asdf"), "is on equal std::strings passes");
    is(std::string("asdf"), "asdf", "is on equal std::string and const char* passes");
    within(1.0, 0.1, 1.001, "within can pass");
    try_within([]{return 1.4;}, 0.1, 1.399, "try_within works");
    about(1.0, 1.001, "about can pass");
    try_about([]{return 1.4;}, 1.4004, "try_about can take functions");
    about(-25, -25.003, "about can take negative numbers");
    doesnt_throw([]{}, "doesnt_throw can pass");
    throws<int>([]{throw (int)3;}, "throws<int> can pass");
    throws_is([]{throw (int)3;}, 3, "throws_is can compare the exception");
    throws_check<int>([]{throw (int)3;}, [](int x){return x==3;}, "throws_check can test the exception");
    struct bad : scary_exception { };
    throws<bad>([]{
        try_ok([]{throw bad{}; return true;}, "Shouldn't reach this");
        fail("Shouldn't reach this");
    }, "bail_out_exception skips normal handlers but is caught by throws<bail_out_exception>()");

    skip("Pretend to skip a test");
    skip(6, "Pretend to skip 6 tests");
    todo("Testing todo (and failures)");
    fail("fail fails");
    todo(2, "Testing numeric todo (and failures)");
    ok(false, "ok on false fails");
    try_ok([]{return false;}, "try_ok can fail");
    todo("Testing block todo (and failures)", [&]{
        is((int)5, (int)3245, "is can fail");
        is_strcmp("asdf", "fdsa", "is_strcmp can fail");
        is_strcmp("sadf", NULL, "is_strcmp fails on single NULL");
        is_strcmp((const char*)NULL, "sadf", "is_strcmp fails on single NULL");
        int nope = -9999;
        is(&heyguys, &nope, "is fails on different pointers");
        is(std::string("sadf"), std::string("qwert"), "is fails on different std::strings");
        within(1.0, 0.1, 1.11, "within can fail");
        try_within([]{return 1.4;}, 0.3, 1, "try_within can fail");
        about(1.0, 1.1, "about can fail");
        doesnt_throw([]{throw std::logic_error("ACK");}, "doesnt_throw catches and fails on exception");
        throws<int>([]{ }, "throws fails when no exception is thrown");
        throws<int>([]{throw std::logic_error("ACK");}, "throws fails on wrong kind of exception");
        throws_check<int>([]{throw (int)3;}, [](int x){return x==5;}, "throws can fail the exception test");
        try_ok([]{
            throw std::logic_error("false");
            return true;
        }, "try_ok catches and fails on exception");
        try_is<int>([]{
            throw std::logic_error("X");
            return 32;
        }, 32, "try_is catches and fails on exception");
    });
});
#endif
#endif

#ifdef TAP_DEFINE_MAIN
int main (int argc, char** argv) {
    tap::allow_testing(argc, argv);
    return 0;
}
#endif
