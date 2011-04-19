// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Licensed under the BSD License, see LICENSE file for details.

#ifndef BABUDB_TEST_H
#define BABUDB_TEST_H

#include "yield/platform/yunit.h"
#include "yield/platform/platform_exception.h"
#include "yield/platform/disk_operations.h"
#include "yield/platform/directory_walker.h"

#define TEST_OUTPUT_DIR "test_out"

class TestCaseTmpDir : public YIELD::TestCase {
public:
	TestCaseTmpDir(const char* short_description, YIELD::TestSuite& test_suite)
		: YIELD::TestCase(short_description, test_suite) {}

	void setUp() {
		if(YIELD::DiskOperations::exists(YIELD::Path(TEST_OUTPUT_DIR) + __short_description))
			YIELD::DiskOperations::rmtree(YIELD::Path(TEST_OUTPUT_DIR) + __short_description);
		try {
			YIELD::DiskOperations::mkdir(YIELD::Path(TEST_OUTPUT_DIR));
		} catch(YIELD::PlatformException& ) {}

		YIELD::DiskOperations::mkdir(YIELD::Path(TEST_OUTPUT_DIR) + __short_description);
	}

	YIELD::Path testPath(const std::string& filename = "") {
		return YIELD::Path(TEST_OUTPUT_DIR) + __short_description + filename;
	}
};

#define EXPECT_EQUAL(stat_a,stat_b) \
	{ if ( !( (stat_a) == (stat_b) ) ) throw YIELD::AssertionException( __FILE__, __LINE__, #stat_a" != "#stat_b ); }
#define EXPECT_TRUE(stat) \
	{ if ( !( (stat) == true ) ) throw YIELD::AssertionException( __FILE__, __LINE__, #stat" != true" ); }
#define EXPECT_FALSE(stat) \
	{ if ( !( (stat) == false ) ) throw YIELD::AssertionException( __FILE__, __LINE__, #stat" != false" ); }


#define TEST_TMPDIR( short_description, TestSuiteName ) \
extern YIELD::TestSuite& TestSuiteName##TestSuite(); \
class short_description##Test : public TestCaseTmpDir \
{ \
public:\
	short_description##Test() : TestCaseTmpDir( #short_description "Test", TestSuiteName##TestSuite()  ) { }\
  void runTest();\
};\
short_description##Test short_description##Test_inst;\
void short_description##Test::runTest()

#endif
