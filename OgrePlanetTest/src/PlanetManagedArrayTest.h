#ifndef __PLANET_MANAGED_ARRAY_TEST__
#define __PLANET_MANAGED_ARRAY_TEST__

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

// Boost unit test frame work includes
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include "OgreMath.h"

// The class to be tested
#include "PlanetManagedArray.h"


namespace OgrePlanet
{
	namespace Test
	{
		// Quick class for testing
		class DummyClass
		{
		public:
			DummyClass(const uint32 i) { mI = i; };
			virtual ~DummyClass() { };
			void setI(uint32 i) { mI = i; };
			uint32 getI() { return mI; };
		private:
			uint32 mI;
		};


		// The test definitions
		class ManagedArrayTest
		{
		public:
			// Constructor
			ManagedArrayTest()
			{
				BOOST_TEST_MESSAGE("ManagedArrayTest()");
			};
			~ManagedArrayTest()
			{
				BOOST_TEST_MESSAGE("~ManagedArrayTest()");
			}
		   
			
			void testAll()
			{
				// Initalise an array as 'empty'
				const uint32 NUM = 100;
				ManagedArray<DummyClass> managedArray(NUM);					
				DummyClass *store[NUM][NUM];
				for (uint32 x=0; x<NUM; x++)
				{
					for (uint32 y=0; y<NUM; y++)
					{
						store[x][y] = NULL;
					}
				}				
							
				// Put some random numbers in random positions
				for (uint32 i=0; i< NUM; i++)
				{
					uint32 x = 0; 
					uint32 y = 0;
					while (store[x][y] != NULL)  // Find free slot
					{
						x = uint32(Math::RangeRandom(0, Real(NUM)));
						y = uint32(Math::RangeRandom(0, Real(NUM)));
					}
					store[x][y] = new DummyClass(i);
					managedArray.add(store[x][y], x, y);
				}

				// Compare results to what they should be
				for (uint32 x=0; x<NUM; x++)
				{
					for (uint32 y=0; y<NUM; y++)
					{
						if (store[x][y] != NULL)
						{
							BOOST_CHECK_EQUAL((managedArray.getWritable(x, y))->getI(), store[x][y]->getI());
						}
					}
				}

				// Clean up
				managedArray.clear();
			};
		};



		// This should really be populated via a boost macro ...
		// The test suite definition
		class ManagedArrayTestSuite : public test_suite
		{
		public:
			ManagedArrayTestSuite() : test_suite("ManagedArrayTestSuite")
			{
				// Create an instance of the test case class
				boost::shared_ptr<ManagedArrayTest> instance (new ManagedArrayTest());
				
				// Create the test classes
				test_case *allTestCase = BOOST_CLASS_TEST_CASE(
					&ManagedArrayTest::testAll, instance);
				
				// Add the test cases to the test suite
				add(allTestCase);
			};
		};
	}
}
#endif
