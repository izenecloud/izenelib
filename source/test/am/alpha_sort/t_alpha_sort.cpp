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
//#define IZENE_LOG
//#include <util/log.h>
#include <string>
#include <time.h>
#include <math.h>
//#include <boost/test/unit_test.hpp>

#include <fstream>
#include <iostream>

//#include <am/external_sort/alpha_sort.hpp>
#include <am/external_sort/multi_pass_sort.hpp>
#include <sys/time.h>
#include <signal.h>

//USING_IZENE_LOG();

using namespace izenelib::am;
using namespace std;

uint32_t error_count = 0;
#define CHECK(f)\
  {                                                \
    if (!(f)){ ++error_count; std::cout<<"ERROR: "<<__FILE__<<": "<<__LINE__<<": "<<__FUNCTION__<<endl;} \
  }
#define ERROR_COUNT {if(error_count>0)cout<<endl<<error_count<<" errors ware found!";else{cout<<"\nNo error detected!\n"}}

// void alpha_sort_check()
// {
  
//   AlphaSort<uint32_t, false, 20000000> alpha;
  
//   alpha.addInputFile("./input");

//   struct timeval tvafter, tvpre;
//   struct timezone tz;
  
//   gettimeofday (&tvpre , &tz);
//   alpha.sort("./output");
//   gettimeofday (&tvafter , &tz);
//   printf( "\nIt takes %f mins to sort 1000000 random data!\n", ((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.);
//   //alpha.printSortedPool();
//   //alpha.cs_printTournamentTree();
//   //alpha.t_checkSortedPool();
//   alpha.t_checkOutputFile();
//     //alpha.printOutputBuffer();
// }


template<class T>
std::vector<char> randstr()
{
  uint32_t len = rand()%100+sizeof(T);
  while(len == sizeof(T))
    len = rand()%100+sizeof(T);

  std::vector<char> str;
  str.reserve(len);
  for (uint32_t i=0; i<len; ++i)
    str.push_back('a'+rand()%26);
  
  return str;
}

void check_multi_sort(uint32_t group_size = 4)
{
  system("rm -f ./tt*");
  struct timeval tvafter, tvpre;
  struct timezone tz;
  typedef uint32_t key_t;
  const uint32_t SIZE = 200000;
  
  MultiPassSort<key_t> sorter("./tt", SIZE*12/500, group_size);

  cout<<"-----------------------------\n";
  std::vector<char> str;
  gettimeofday (&tvpre , &tz);
  for (uint32_t i=0; i<SIZE; ++i)
  {
    str = randstr<key_t>();
    sorter.add_data(str.size(), str.data());
    cout<<"\rAdd data: "<<(double)i/SIZE*100.<<"%"<<std::flush;
  }
  gettimeofday (&tvafter , &tz);
  printf( "\nIt takes %f minutes to add %d random data!\n",
          ((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000.)/60000,
          SIZE);

  gettimeofday (&tvpre , &tz);
  sorter.sort();
  gettimeofday (&tvafter , &tz);
  printf( "\nIt takes %f minutes to sort %d random data!\n",
          ((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000.)/60000,
          SIZE);

  gettimeofday (&tvpre , &tz);
  sorter.output();
  gettimeofday (&tvafter , &tz);
  printf( "\nIt takes %f minutes to output %d random data!\n",
          ((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000.)/60000,
          SIZE);

  char* data;
  uint8_t len;
  key_t last = 0;

  CHECK(sorter.begin());
  while (sorter.next_data(len, &data))
  {
    CHECK(last<=*(key_t*)data);
  }
}

int main()
{
  //alpha_sort_check();
  //check_multi_sort(200);
  check_multi_sort();
}



//BOOST_AUTO_TEST_SUITE_END()


