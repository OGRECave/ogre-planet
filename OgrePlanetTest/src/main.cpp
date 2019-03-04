/*
 * OgrePlanet dynamic level of detail for planetary rendering
 * Copyright (C) 2008 Beau Hardy 
 * http://www.gamepsychogony.co.nz
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in 
 * the Software without restriction, including without limitation the rights to 
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
 * of the Software, and to permit persons to whom the Software is furnished to do 
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 */


// Show what .lib file is being linked to during build
#define BOOST_LIB_DIAGNOSTIC

// Show compiler / environment et al.
#define BOOST_TEST_BUILD_INFO yes

// Basic memory leak detection
#define BOOST_TEST_MEMORY_LEAK 1

// Post build command in IDE runs built .exe
// "BOOST_LOG_LEVEL all" Catches all errors during post build execution
// Usually this allows double click to go to file/line
// Note in team / automation cases the built .exe can be run with logging
// filename.exe --report_level=detailed --report_format=XML
#define BOOST_TEST_LOG_LEVEL all
#define BOOST_TEST_REPORT_LEVEL detailed
#define BOOST_TEST_OUTPUT_FORMAT HRF
#define BOOST_TEST_SHOW_PROGRESS yes

// Run test cases in random order
#define BOOST_TEST_RANDOM 1

// Force exit level success for test execution
// Lets failed 'post build tests' be debugged in IDE
#define BOOST_TEST_RESULT_CODE 1


// BOOST_WARN - exec continues, no error count
// BOOST_CHECK - exec continues, error count++
// BOOST_REQUIRE - exec stops


// Boost unit test framework includes
#include <boost/test/unit_test_suite.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

// The test cases and test suites
// Note included .cpp not .h files to get around problems with above defines and recompiles
#include "PlanetManagedArrayTest.h"

// Kludge for Ogre logging 'memory leaks' due to open log and ability to call functions that log
#include "mainOgreLog.h"



// IMPORTANT test suite *init_unit_test_suite MUST be defined in namespace boost::unit_test
// When linked to correct library, this replaces int main(int argc, char* argv[])
// Note also use of "char *argv[]" rather than "const char **" to get link working
// namespace boost { namespace unit_test { 'init_unit_test_suite(...)' }} NO GO either ...
// Yes, this cost me HOURS!
using namespace boost::unit_test;

// test program entry point - generates "int main(int argc, char *argv[])"
test_suite* init_unit_test_suite(int argc, char* [])  
{
	// Create the top test suite
	test_suite* suite(BOOST_TEST_SUITE("Master Suite"));

	// Turn on the Ogre log
	suite->add(new OgrePlanet::Test::LogStartTestSuite());


	// add test suites to the top test suite
	suite->add(new OgrePlanet::Test::ManagedArrayTestSuite());

	
	// Turn off the Ogre log
	suite->add(new OgrePlanet::Test::LogStopTestSuite());

	return suite;
};
