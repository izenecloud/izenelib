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
#define IZENE_LOG
#include <util/log.h>
#include <string>
#include <time.h>
#include <math.h>
//#include <boost/test/unit_test.hpp>

#include <fstream>
#include <iostream>

#include <am/external_sort/alpha_sort.hpp>

#include <signal.h>

USING_IZENE_LOG();

using namespace izenelib::am;
using namespace std;

//BOOST_AUTO_TEST_SUITE( alpha_sort_suite )


//BOOST_AUTO_TEST_CASE(input_check )
int main()
{
  AlphaSort<> alpha;
  
  alpha.addInputFile("./input");
  
  clock_t start, finish;
  start = clock();
  alpha.sort("./output");
  finish = clock();
  
  printf( "\nIt takes %f seconds to insert 1000000 random data!\n", (double)(finish - start) / CLOCKS_PER_SEC);
  //alpha.printSortedPool();
  //alpha.cs_printTournamentTree();
  //alpha.t_checkSortedPool();
  alpha.t_checkOutputFile();
    //alpha.printOutputBuffer();
  
}



//BOOST_AUTO_TEST_SUITE_END()


