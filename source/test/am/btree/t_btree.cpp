/// @file   t_UString.cpp
/// @brief  A test unit for checking if all interfaces is 
///         available to use.
/// @author Do Hyun Yun 
/// @date   2008-07-11
///
///  
/// @brief Test all the interfaces in UString class.
///
/// @details
/// 
/// ==================================== [ Test Schemes ] ====================================
///
///
/// -# Tested basic part of UString according to the certain scenario with simple usage.\n
/// \n 
///     -# Create three UString variables in different ways : Default Initializing, Initializing with another UString, and initialize with stl string class.\n\n
///     -# Check attributes of some characters in UString using is_____Char() interface. With this interface, it is possible to recognize certain character is alphabet or number or something.\n\n
///     -# Get attribute of certain characters in UString using charType() interface.\n\n
///     -# Change some characters into upper alphabet or lower alphabet using toUpperChar() and toLowerChar(), and toLowerString() which changes all characters in UString into lower one.\n\n
///     -# With given pattern string, Get the index of matched position by using find(). \n\n
///     -# Create the sub-string using subString() with the index number which is the result of find().\n\n
///     -# Assign string data in different ways using assign(), format() interfaces and "=" "+=" operators.\n\n
///     -# Export UString data into stl string class according to the encoding type.\n\n
///     -# Check size, buffer size, and its length. Clear string data and re-check its information including empty().\n\n
/// \n
/// -# Tested all the interfaces by using correct and incorrect test sets.

#include <am/btree/BTreeFile.h>
#include <boost/memory.hpp>
#include <boost/test/unit_test.hpp>
#include <math.h>
#include <string>
#include <time.h>

#define SIZE 1000000

BOOST_AUTO_TEST_SUITE( t_BTreeFile_suite )

int degree = 1024;

izenelib::am::BTreeFile<int, string> tb("sdb.dat");

BOOST_AUTO_TEST_CASE(Initialization) {
	tb.setDegree(degree);
	tb.setPageSize(60);
	tb.open();
}

BOOST_AUTO_TEST_CASE(Insertion_check) {

	//std::string s = "Kevin";

	clock_t start, finish;
	start = clock();
	for (int i=0; i<SIZE; i++) {
		char p[20];
		sprintf(p, "%08d", i);
		string str = p;
		tb.insert(i, str);
	}
	tb.flush();
	finish = clock();
	printf("\nIt takes %f seconds to insert %d random data!\n", (double)(finish
			- start) / CLOCKS_PER_SEC, SIZE);
}

BOOST_AUTO_TEST_CASE(Searching_check) {

	clock_t start, finish;
	int c, b;
	c=b=0;

	start = clock();

	for (int i=0; i<SIZE; i++) {
		if (tb.find(i) )
			c++;
		else
			b++;
	}
	tb.flush();
	finish = clock();
	printf(
			"\nIt takes %f seconds to find %d random data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, SIZE, c, b);

	//  tb.display(std::cout)
}

BOOST_AUTO_TEST_CASE(Delete_check) {

	clock_t start, finish;
	int c, b;
	c=b=0;

	start = clock();
	for (int i=0; i<SIZE/2; i++) {
		if (tb.del(i) != 0)
			c++;
		else
			b++;
	}
	tb.flush();
	finish = clock();
	printf(
			"\nIt takes %f seconds to delete %d random data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, SIZE/2, c, b);

}

BOOST_AUTO_TEST_SUITE_END()
