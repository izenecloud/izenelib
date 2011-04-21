// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_YUNIT_H
#define YIELD_PLATFORM_YUNIT_H

#include "yield/platform/assert.h"

#include <vector>
#include <string>
#include <iostream>
#include <exception>

#include "yield/platform/debug.h" // Should be last to get DebugBreak on Windows


namespace YIELD
{
	class TestCase;

	class TestSuite : public std::vector<TestCase*>
	{
	public:
		void addTest( TestCase* test_case ) { push_back( test_case ); }
	};


	class TestCase
	{
	public:
		TestCase( const char* short_description ) : __short_description( short_description ) { }
		TestCase( const char* short_description, TestSuite& __test_suite ) : __short_description( short_description ) { __test_suite.addTest( this ); }

		virtual void setUp() { }
		virtual void runTest() = 0;
		virtual void tearDown() { }
		virtual const char* shortDescription() { return __short_description.c_str(); }

	protected:
		friend class TestRunner;
		std::string __short_description;
	};


	class TestRunner
	{
	public:
		int run( TestSuite& test_suite )
		{
			int ret_code = 0;

			for ( TestSuite::iterator i = test_suite.begin(); i != test_suite.end(); i++ )
			{
				bool called_runTest = false, called_tearDown = false;

				try
				{
					std::cerr << ( *i )->__short_description;
					( *i )->setUp();
					called_runTest = true;
					( *i )->runTest();
					called_tearDown = true;
					( *i )->tearDown();
					std::cerr << ": passed" << std::endl;
					continue;
				}
				catch ( YIELD::AssertionException& exc )
				{
					std::cerr << " failed: " << exc.what() << std::endl;
				}
				catch ( std::exception& exc )
				{
					std::cerr << " threw unknown exception: " << exc.what() << std::endl;
				}
				catch ( ... )
				{
					std::cerr << " threw unknown non-exception" << std::endl;
				}

				if ( called_runTest && !called_tearDown )
					try { ( *i )->tearDown(); } catch ( ... ) { }

				ret_code |= 1;
			}

			return ret_code;
		}
	};

#define TEST_SUITE( TestSuiteName ) \
YIELD::TestSuite& TestSuiteName##TestSuite() { static YIELD::TestSuite* ts = new YIELD::TestSuite(); return *ts; } \
class TestSuiteName##TestSuiteDest { public: ~TestSuiteName##TestSuiteDest() { delete &TestSuiteName##TestSuite(); }}; \
TestSuiteName##TestSuiteDest TestSuiteName##TestSuiteDestObj;

#define DECLARE_TEST_SUITE( TestSuiteName ) \
extern YIELD::TestSuite& TestSuiteName##TestSuite();

#define TESTEX( TestCaseName, TestCaseParentClassName, TestSuiteName ) \
extern YIELD::TestSuite& TestSuiteName##TestSuite(); \
class TestCaseName##Test : public TestCaseParentClassName \
{ \
public:\
	TestCaseName##Test() : TestCaseParentClassName( #TestCaseName "Test", TestSuiteName##TestSuite() ) { }\
  void runTest();\
};\
TestCaseName##Test TestCaseName##Test_inst;\
void TestCaseName##Test::runTest()

#define TEST( TestCaseName, TestSuiteName ) TESTEX( TestCaseName, TestCase, TestSuiteName )


#ifdef YIELD_BUILDING_STANDALONE_TEST
#define TEST_MAIN( TestSuiteName ) \
	int main( int argc, char** argv ) { return YIELD::TestRunner().run( TestSuiteName##TestSuite() ); }
#else
#define TEST_MAIN( TestSuiteName)
#endif
};

#endif
