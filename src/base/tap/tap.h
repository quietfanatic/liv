
 // A TAP outputting test library for C++
 //
 // Instructions:
 //
 // 1. Declare Testers at the bottom of each cpp file, or wherever you want.  They
 //     should look something like:
 //
 //     #include "path/to/tap.h"
 //     static tap::Tester universe_tests ("universe/universe.cpp" [](){
 //         using namespace tap;
 //         plan(3);
 //         ok(init_universe(), "Everything starts up right");
 //         is(get_answer(), 42, "Just in case");
 //         within(entropy(), 0.1, 0, "Not too hot");
 //     });
 //
 //     Make sure you provide a unique name for each tester.  They don't have to
 //     be the filename of the file in which they reside.
 //
 // 2. There are two ways to run the tests.
 //     A) At the front of your main function, put
 //
 //         tap::allow_testing(argc, argv);
 //
 //         This will cause your program to respond to those command-line arguments.
 //         If you give your program "--test universe/universe.cpp" as arguments, it
 //         will run the test you installed with that name *and then exit*.  If you give
 //         it "--test" without arguments, it will print a list of command lines in the
 //         form of "./my_program --test universe/universe.cpp" for each test you've
 //         declared.  Giving a const char* as a third argument will make it look for
 //         something else instead of "--test".  If you pass an empty string as the third
 //         argument, then running your program with a single argument or no arguments
 //         will produce the above behavior.
 //
 // 3. Compile tap.cpp and link tap.o with your program, or if tap is a shared
 //     library somewhere, link to it.
 //
 // 4. To run all the test through a harness, do
 //
 //     ./my_program --test | prove -e "./my_program --test" -
 //
 // 5. To compile with tests disabled for release, define TAP_DISABLE_TESTS.  If this is
 //     defined, no tests will be registered, and ideally your linker will discard all
 //     testing code.

///// BOILERPLATE

#pragma once

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>

#include <iostream>

namespace tap {
using namespace std;

///// Tester

#ifndef TAP_DISABLE_TESTS

 // This is the struct you use to declare a set of tests.
struct TestSet {
     // The constructor will register the test at init-time.
    TestSet (const std::string& name, const std::function<void()>& code);
};

#else

 // Stub struct for when TAP_DISABLE_TESTS is defined.
namespace {
struct TestSet {
    TestSet (const std::string&, const std::function<void()>&) { }
};
}

#endif

///// Testing functions

 // Do this at the beginning of your testing.  Give it as an argument the
 //  number of tests you plan to run.  If you run a different number, the
 //  test set is considered to have failed.
void plan (unsigned num_tests);

 // Alternatively, do this at the end of your testing.
void done_testing ();

 // Run a test.  If succeeded is true, the test is successful, otherwise it
 //  is a failure.
bool ok (bool succeeded, const std::string& name = "");
 // The try_* versions of testing functions fail if the code throws an exception.
 //  Otherwise, they behave like the non-try versions with the returned result.
bool try_ok (const std::function<bool()>& code, const std::string& name = "");

 // Run a test that succeeds if got == expected (with overloaded operator ==).
 //  If the test failed, it will try to tell you what it got vs. what it expected.
 // Will fail if the == operator throws an exception.
 // You probably know that you shouldn't use == to compare floating point numbers,
 //  so for those, look at within() and about().
 // As a special case, you can use is() with const char* and it'll do a strcmp (with
 //  NULL checks).
template <class A, class B>
bool is (const A& got, const B& expected, const std::string& name = "");
template <class A, class B>
bool try_is (const std::function<A()>& code, const B& expected, const std::string& name = "");
 // You can call the special case directly if you want.
bool is_strcmp(const char* got, const char* expected, const std::string& name = "");
bool try_is_strcmp(const std::function<const char*()>& code, const char* expected, const std::string& name = "");

 // Unlike is, isnt isn't that useful, but at least it catches exceptions in the != operator.
template <class A, class B>
bool isnt (const A& got, const B& unexpected, const std::string& name = "");
template <class A, class B>
bool try_isnt (const std::function<A()>& code, const B& unexpected, const std::string& name = "");
bool isnt_strcmp(const char* got, const char* unexpected, const std::string& name = "");
bool try_isnt_strcmp(const std::function<const char*()>& code, const char* unexpected, const std::string& name = "");

 // There isn't enough of a use case for isnt() to justify coding it, since unlike
 // is(), printing out got and expected isn't particularly useful.
 // Just use ok(a != b, "a isn't b");

 // Tests that got is within +/- range of expected.
bool within (double got, double range, double expected, const std::string& name = "");
bool try_within (const std::function<double()>& code, double range, double expected, const std::string& name = "");
 // Tests that got is within a factor of .001 of expected.
static inline bool about (double got, double expected, const std::string& name = "") {
    return within(got, expected*0.001, expected, name);
}
static inline bool try_about (const std::function<double()>& code, double expected, const std::string& name = "") {
    return try_within(code, expected*0.001, expected, name);
}

 // Tests that code throws an exception of class Except.  If a different kind of
 //  exception is thrown, the test fails.
template <class E = std::exception>
bool throws (const std::function<void()>& code, const std::string& name = "");

 // Like above, but fails if the thrown exception does not == expected.
template <class E = std::exception>
bool throws_is (const std::function<void()>& code, const E& expected, const std::string& name = "");

 // Like above, but fails if the thrown exception does not satisfy check.
template <class E = std::exception>
bool throws_check (const std::function<void()>& code, const std::function<bool(const E&)>& check, const std::string& name = "");

 // Succeeds if no exception is thrown.
bool doesnt_throw (const std::function<void()>& code, const std::string& name = "");

 // Automatically pass a test with this name.  Only resort to this if you can't
 //  make your test work with the other testing functions.
bool pass (const std::string& name = "");
 // Likewise with fail.
bool fail (const std::string& name = "");

 // Alias for doesnt_throw
static inline bool try_pass (const std::function<void()>& code, const std::string& name = "") {
    return doesnt_throw(code, name);
}

 // Mark the next num tests as todo.  You must still run the tests.  If only
 //  todo tests fail, the test set is still considered successful.
void todo (unsigned num, const std::string& excuse = "");
 // Just todo one test.
static inline void todo (const std::string& excuse = "") {
    todo(1, excuse);
}
 // The block form marks as todo every test that runs inside it.  It can be safely
void todo (const std::string& excuse, const std::function<void()> code);
static inline void todo (const std::function<void()> code, const std::string& excuse = "") {
    todo(excuse, code);
}

 // Declare that you've skipped num tests.  You must not still run the tests.
void skip (unsigned num, const std::string& excuse = "");
 // Just skip one test.
static inline void skip (const std::string& excuse = "") {
    skip(1, excuse);
}

///// DIAGNOSTICS

 // Tap will use this to print strings to stdout.  By default, this is
 //  fputs(s.c_str(), stdout);
void set_print (void(*)(const std::string&));

 // Convert an arbitrary item to a string.  Feel free to overload this for your
 //  own types.  Throwing exceptions from show() may cause duplicate test failures.
template <class T>
struct Show {
    std::string show (const T&);
};

 // Print a message as diagnostics.  Should not contain newlines.
void diag (const std::string& message);

///// UH-OH

 // When everything is wrong and you can't even continue testing.  Immediately fails
 //  the whole test set and calls exit(1).
void BAIL_OUT (const std::string& reason = "");

 // Testing functions normally catch exceptions, but they won't catch ones that
 //  inherit from this (unless it's a throws<>() and the exception matches it).
struct scary_exception : std::exception { };

///// RUNNING TESTS

 // Do this in main to allow command-line testing.
void allow_testing (int argc, char** argv, const char* test_flag = "--test");

 // To run a test set manually, do this.  It will not exit unless BAIL_OUT is called.
void run_test (const std::string& name);
 // To list the tests manually, do this.  It will print test set names to stdout.
void list_tests ();

 // Copies of the parameters passed to allow_testing that you can access from
 //  your tests.  These are not available if you directly call run_test.
extern int argc;
extern char** argv;

///// IMPLEMENTATION DETAILS

namespace internal {
    std::string type_name (const std::type_info& type);

    bool fail_on_throw (const std::function<bool()>&, const std::string& name);

    template <class A, class B>
    void diag_unexpected (const A& got, const B& expected);

    void diag_didnt_throw (const std::type_info& type);
    void diag_wrong_exception (const std::exception& got, const std::type_info& type);
    void diag_wrong_exception_nonstandard (const std::type_info& type);
    template <class E>
    void diag_exception_failed_check (const E& got);

    std::string show_ptr (void*);
}

///// TEMPLATE IMPLEMENTATIONS

template <class A, class B>
bool is (const A& got, const B& expected, const std::string& name) {
    return internal::fail_on_throw([&]{
        if (got == expected) {
            return pass(name);
        }
        else {
            fail(name);
            internal::diag_unexpected(got, expected);
            return false;
        }
    }, name);
}
template <class A, class B>
bool try_is (const std::function<A()>& code, const B& expected, const std::string& name) {
    return internal::fail_on_throw([&]{
        const A& got = code();
        if (got == expected) {
            return pass(name);
        }
        else {
            fail(name);
            internal::diag_unexpected(got, expected);
            return false;
        }
    }, name);
}
static inline bool is (const char* got, const char* expected, const std::string& name) {
    return is_strcmp(got, expected, name);
}
static inline bool try_is (const std::function<const char*()>& code, const char* expected, const std::string& name) {
    return try_is_strcmp(code, expected, name);
}

template <class A, class B>
bool isnt (const A& got, const B& unexpected, const std::string& name) {
    return internal::fail_on_throw([&]{
        return ok(got != unexpected, name);
    }, name);
}
template <class A, class B>
bool try_isnt (const std::function<A()>& code, const B& unexpected, const std::string& name) {
    return internal::fail_on_throw([&]{
        return try_ok(code() != unexpected, name);
    }, name);
}
static inline bool isnt (const char* got, const char* unexpected, const std::string& name) {
    return isnt_strcmp(got, unexpected, name);
}
static inline bool try_isnt (const std::function<const char*()>& code, const char* unexpected, const std::string& name) {
    return try_isnt_strcmp(code, unexpected, name);
}

template <class E>
bool throws (const std::function<void()>& code, const std::string& name) {
    try { code(); return true; }
    catch (const E& e) {
        return pass(name);
    }
    catch (const scary_exception& e) { throw; }
    catch (const std::exception& e) {
        fail(name);
        internal::diag_wrong_exception(e, typeid(E));
        return false;
    }
    catch (...) {
        fail(name);
        internal::diag_wrong_exception_nonstandard(typeid(E));
        return false;
    }
}

template <class E>
bool throws_is (const std::function<void()>& code, const E& expected, const std::string& name) {
    try { code(); return true; }
    catch (const E& e) {
        if (e == expected) {
            return pass(name);
        }
        else {
            fail(name);
            internal::diag_unexpected(e, expected);
            return false;
        }
    }
    catch (const scary_exception& e) { throw; }
    catch (const std::exception& e) {
        fail(name);
        internal::diag_wrong_exception(e, typeid(E));
        return false;
    }
    catch (...) {
        fail(name);
        internal::diag_wrong_exception_nonstandard(typeid(E));
        return false;
    }
}

template <class E>
bool throws_check (const std::function<void()>& code, const std::function<bool(const E&)>& check, const std::string& name) {
    try { code(); return true; }
    catch (const E& e) {
        if (check(e)) {
            return pass(name);
        }
        else {
            fail(name);
            internal::diag_exception_failed_check(e);
            return false;
        }
    }
    catch (const scary_exception& e) { throw; }
    catch (const std::exception& e) {
        fail(name);
        internal::diag_wrong_exception(e, typeid(E));
        return false;
    }
    catch (...) {
        fail(name);
        internal::diag_wrong_exception_nonstandard(typeid(E));
        return false;
    }
}

///// Default show

template <class T>
std::string Show<T>::show (const T& v) {
    if constexpr (std::is_same_v<T, bool>) {
        return v ? "true" : "false";
    }
    else if constexpr (std::is_same_v<T, char>) {
        return std::string("'") + v + "'";
    }
    else if constexpr (std::is_same_v<T, const char*>) {
        return v ? "\"" + std::string(v) + "\"" : "nullptr";
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        return "\"" + v + "\"";
    }
    else if constexpr (std::is_same_v<T, std::string_view>) {
        return "\"" + v + "\"";
    }
    else if constexpr (std::is_same_v<T, std::nullptr_t>) {
        return "nullptr";
    }
    else if constexpr (std::is_same_v<T, std::exception>) {
        return "exception of type " + internal::type_name(typeid(v)) + ": " + v.what();
    }
    else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(v);
    }
    else if constexpr (std::is_pointer_v<T>) {
        return internal::show_ptr((void*)v);
    }
    else if constexpr (std::is_enum_v<T>) {
        return "(enum value) " + std::to_string(std::underlying_type_t<T>(v));
    }
    else {
        return "(unprintable object of type " + internal::type_name(typeid(T)) + ")";
    }
}

namespace internal {
    template <class A, class B>
    void diag_unexpected(const A& got, const B& expected) {
        diag("Expected " + Show<B>().show(expected));
        diag("     got " + Show<A>().show(got));
    }
    template <class E>
    void diag_exception_failed_check(const E& got) {
        diag("Exception failed the check");
        diag("     Got " + Show<E>().show(got));
    }
}

}  // namespace tap

