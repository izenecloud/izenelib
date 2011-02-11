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

#include <boost/memory.hpp>
#include <am/skip_list/skip_list.hpp>
#include <string>
#include <time.h>
#include <math.h>
#include <boost/test/unit_test.hpp>
#include <time.h>

#define SIZE 1000000

BOOST_AUTO_TEST_SUITE( t_SkipList_suite1 )

BOOST_AUTO_TEST_CASE(Insertion_check )
{
  izenelib::am::SkipList< std::string > tb;

  class Ele
  {
  public:
    Ele()
    {
    }
    
    Ele(int a,const std::string& b )
    {
      k =a;
      str = b;
    }
    
    std::string str;
    int k;
    
  };
  
    
  std::string s = "Kevin";
  Ele* ele = new Ele[SIZE];
  for (int i=0; i<SIZE; i++)
  {
    if (i%100 == 0)
      s.append("_");

    ele[i]=  Ele(rand()%SIZE, s);
    
  }

  clock_t start, finish;
  start = clock();
  for (int i=0; i<SIZE; i++)
  {
    tb.insert(ele[i].k, ele[i].str);
  }
  finish = clock();
  printf( "\nIt takes %f seconds to insert 1000000 random data!\n", (double)(finish - start) / CLOCKS_PER_SEC );
  
  //  tb.display(std::cout);
   
}


BOOST_AUTO_TEST_CASE(Searching_check )
{
  izenelib::am::SkipList< std::string > tb;
  std::string s = "Kevin";
  for (int i=1; i<=SIZE; i++)
  {
    if (i%100 == 0)
      s.append("_");

    tb.insert(rand()%SIZE, s);
    
  }

  clock_t start, finish;
  int c ,b;
  c=b=0;
  int r[SIZE];
  
  for (int i=0; i<SIZE; i++)
  {
    r[i] = rand()%SIZE;
  }
  
  start = clock();
  
  for (int i=0; i<SIZE; i++)
  {
    if (tb.find(r[i])!=0)
      c++;
    else
      b++;
  }

  finish = clock();
  printf( "\nIt takes %f seconds to find 1000000 random data! %d data found, %d data lost!\n",
          (double)(finish - start) / CLOCKS_PER_SEC,
          c,
          b);
  
  //  tb.display(std::cout)
}

BOOST_AUTO_TEST_CASE(Delete_check )
{
  izenelib::am::SkipList< std::string > tb;
  std::string s = "Kevin";
  for (int i=1; i<=SIZE; i++)
  {
    if (i%100 == 0)
      s.append("_");

    tb.insert(rand()%SIZE, s);
    
  }

  clock_t start, finish;
  int c ,b;
  c=b=0;
  int r[SIZE];
  
  for (int i=0; i<SIZE; i++)
  {
    r[i] = rand()%SIZE;
  }
  
  start = clock();
  
  for (int i=0; i<SIZE; i++)
  {
    if (tb.del(r[i])!=0)
      c++;
    else
      b++;
  }

  finish = clock();
  printf( "\nIt takes %f seconds to delete 1000000 random data! %d data found, %d data lost!\n",
          (double)(finish - start) / CLOCKS_PER_SEC,
          c,
          b);

}

BOOST_AUTO_TEST_CASE(Allocator_check )
{
  clock_t start, finish;
  std::string* p[SIZE];
  start = clock();
  for (int j=0; j<32; j++)
  for (int i=0; i<SIZE; i++)
  {
    p[i] = new std::string();
    delete p[i];
    
    
  }

  finish = clock();
  printf( "\nNEW: It takes %f seconds to new 1000000 object\n",
          (double)(finish - start) / CLOCKS_PER_SEC);


  boost::scoped_alloc alloc_;
  start = clock();
  for (int j=0; j<32; j++)
  for (int i=0; i<SIZE; i++)
  {
    p[i] = BOOST_NEW(alloc_, std::string);
    
  }
  finish = clock();
  printf( "\nBOOST_NEW: It takes %f seconds to new 1000000 object\n",
          (double)(finish - start) / CLOCKS_PER_SEC);
  
  
}

BOOST_AUTO_TEST_SUITE_END()
