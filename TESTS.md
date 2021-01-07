# Unit Tests and NSWConfiguration
Ideally, for every new public method a code change introduces, unit tests should be written. Sometimes an entirely new
test module will have to be created. This document aims to provide a guide and some style specifications for the NSWC
test suite, whether one is writing unit tests for an already-existing functionality, or for an entirely new one.

## Creating a Test Module
Test modules correspond to a single class, and aim to test their matching class' public interface.
Creating a testing module is fairly simple; all that is needed is a `.cpp` file and some preprocessor fun:
1. `#define BOOST_TEST_MODULE <test_module_name>`: define a test module, with name "<test_module_name>".
2. `#define BOOST_TEST_DYN_LINK`: indicate the Boost.Test shared object is dynamically linked to the program.
3. `#include <boost/test/unit_test.hpp>`: include the Boost.Test main header.

Note that steps 1 and 2 _must_ be done before the inclusion of any Boost.Test headers.

Some style guidelines for test modules:
* Test module source files should go in the `test/` directory.
* Test module filenames should be `test_X.cpp`, where "X" is the name of the class they're testing.
* Test module names (declared with the `BOOST_TEST_MODULE` #define) should be the name of the class
the module corresponds to.

All this is automated in the `test/create_test_module.sh` bash script. Run it from within the `test/` directory, giving
it the name of the class you're testing as its argument. It'll create a test `.cpp` file template for you.

## Writing Unit Tests
Creating a test case is made fairly simple with the `BOOST_AUTO_TEST_CASE` macro. For example, creating a test case
named "<test_cast_name>" is a simple `BOOST_AUTO_TEST_CASE(<test_case_name>)`. Inside the test case, there are various
checks that can be made, from a simple assert to a check that a specific exception with a specific error message is
thrown. Some common ones are `BOOST_TEST`, `BOOST_CHECK_THROW`, and `BOOST_CHECK_EXCEPTION`. For further information,
refer to the Boost Testing Framework specification.

Some style guidelines for test cases:
* Test case names should follow the format \[UnitOfWork\]\_\[StateUnderTest\]\_\[ExpectedBehavior\]. This pushes test
writers to keep test cases well-defined, unlike a simple \[FunctionName\]_test.
* Test methods should be short and succinct; there is no hard limit on SLOC count, but ~10 seems like a pretty good rule
of thumb.
* Try to write test cases for every possible behavior of the function. For example, a `double sqrt(int x)` function
which throws `std::invalid_argument` when given a negative integer, should have two test cases:
`Sqrt_PositiveInput_ReturnsSquareRoot` and `Sqrt_NegativeInput_ThrowsInvalidArgument`.

## Adding Test Executables
In order to actually run the test modules, CMake has to be aware of them. Adding a new module to the build system
requires the following steps (in CMakeLists.txt):
1. Add a new executable, with `tdaq_add_executable(...)`. _Make sure the executable name starts with test\__.
Link against `Boost::unit_test_framework`, and with whatever else is needed, using the `LINK_LIBRARIES` directive.
2. Add the executable name, with the `test_` prefix removed, to the `NSWCONFIG_TESTS` list (in the `set(...)` call).
3. If the tests requires an additional file, such as a configuration json, make sure to include it with
`configure_file`: `configure_file(test/<filename> <filename> COPYONLY)`.

## Running Test Executables
After building the tests, either through their specific target or by a complete NSWConfiguration build, cd to the
NSWConfiguration directory **inside the build directory**. From there, one can either run `ctest`, which will
attempt to run all test executables (skipping those which are not built), or individual executables
(e.g. `./test_vmmconfig`).

In order to get more information on the tests being run, one can run an individual test executable with
`--log_level=all`, which will print anything and everything test-related.