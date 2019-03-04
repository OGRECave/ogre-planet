#ifndef __MAIN_OGRE_LOG__
#define __MAIN_OGRE_LOG__

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



// Messy Ogre log for use with boost.test
// Need to turn on Ogre logging on or things that try to log crash
// Need to clean up log before master suite exit or 'memory leaks' are reported
#include <boost/test/unit_test_suite.hpp>

#include "OgreLogManager.h"

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

namespace OgrePlanet
{
	namespace Test
	{
		using namespace Ogre;	
		// Classes to hold static references to log manager
		class LogStartTest
		{
		public:
			LogStartTest() { };
			~LogStartTest() { };
			void testStartLog()
			{

				if (Ogre::LogManager::getSingletonPtr() == NULL)
				{
					mLogMgr = new LogManager();
					mLog = mLogMgr->createLog("PsuedoLog.log", true, true, true);
					mLog->logMessage("Ogre Log started");
				}
			}
			static Log *mLog;
			static LogManager *mLogMgr;
		};
		Log *LogStartTest::mLog = NULL;
		LogManager *LogStartTest::mLogMgr = NULL;
		
		class LogStopTest
		{
		public:
			LogStopTest() { };
			~LogStopTest() { };
			void testStopLog()
			{
				using namespace Ogre;
				if (LogStartTest::mLogMgr != NULL)
				{
					LogManager::getSingleton().logMessage("Ogre Log about to stop");
					LogStartTest::mLogMgr->destroyLog(LogStartTest::mLog);
					delete LogStartTest::mLogMgr;
					LogStartTest::mLogMgr = NULL;
				}
			}
		};

		// Suite wrappers for logging
		class LogStartTestSuite : public test_suite
		{
		public:
			LogStartTestSuite() : test_suite("LogStartTestSuite")
			{
				// Create an instance of the test case class
				boost::shared_ptr<LogStartTest> instance (new LogStartTest());
				
				// Create the test classes
				test_case *startLogTestCase = BOOST_CLASS_TEST_CASE(
					&LogStartTest::testStartLog, instance);
				
				// Add the test cases to the test suite
				add(startLogTestCase);
			};
		};


		// Suite wrappers for logging
		class LogStopTestSuite : public test_suite
		{
		public:
			LogStopTestSuite() : test_suite("LogStopTestSuite")
			{
				// Create an instance of the test case class
				boost::shared_ptr<LogStopTest> instance (new LogStopTest());
				
				// Create the test classes
				test_case *stopLogTestCase = BOOST_CLASS_TEST_CASE(
					&LogStopTest::testStopLog, instance);
				
				// Add the test cases to the test suite
				add(stopLogTestCase);
			};
		};
	}
}
#endif
