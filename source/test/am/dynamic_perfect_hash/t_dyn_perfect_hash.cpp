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
//#include <util/log.h>
#include <am/dynamic_perfect_hash/dynamic_perfect_hash.hpp>
#include <string>
#include <time.h>
#include <math.h>
#include <boost/test/unit_test.hpp>
#include <time.h>
#include <fstream>
#include <iostream>

//USING_IZENE_LOG();

BOOST_AUTO_TEST_SUITE( t_dyn_perfect_hash_suite )

using namespace izenelib::am;

void readDict(const string& dict, vector<string>& v)
{
  std::ifstream f;

  f.open (dict.c_str(), std::ifstream::in);
  if (f.fail())
  {
    std::cout<<"Can't open the dictinary file! Please check the file name: "<<dict<<std::endl;
    return;
  }
    
  // get length of file:
  f.seekg (0, std::ios::end);
  std::size_t length = f.tellg();
  f.seekg (0, std::ios::beg);

  // allocate memory:
  char* buffer = new char [length];

  // read data as a block:
  f.read (buffer,length);
  f.close();
  //cout<<buffer;
  std::string bf(buffer);
  std::size_t i=0;
  std::size_t start = 0;
  
  for (; i<length; i++)
  {
    if (bf[i]==' '|| bf[i]=='\n')
    {
        
      if(start+1 == i && (bf[start]==' '||bf[start]=='\n'))
      {
        start = i+1;
        continue;
      }

      std::string str = bf.substr(start, i-start);

      v.push_back(str);
      start = i+1;
      i++;
                
    }
  }

  //cout<<dicTermStr_;
  
  delete buffer;
  
}

BOOST_AUTO_TEST_CASE(dp_hash_numeric_insertion_check )
{
  #define SIZE 1000000
  
  vector<uint64_t> v;
  for (int i=0; i<SIZE; i++)
  {
    v.push_back(rand());
  }
  DynamicPerfectHash<string, string> dpp;
  DynamicPerfectHash<> dp;
  
  clock_t start, finish;
    
  start = clock();
  for (size_t i=0; i<v.size(); i++)
  {
    dp.insert(v[i], v[i]/10);
  }
  finish = clock();
  printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, v.size());
  cout<<"count_: "<<dp.getCount()<<"  update_count_: "<<dp.getUpdateCount()<<"  size: "<<dp.getSize()<<endl;;
  cout<<dp.save("./data.k", "./data.v")<<" saved!\n";

  //cout<<dp;
  
  DynamicPerfectHash<> dd;
  cout<<dd.load("./data.k", "./data.v")<<" loaded!\n";;
  //cout<<dd;
  start = clock();
  for (size_t i=0; i<v.size(); i++)
  {
    if(*dp.find(v[i])!= v[i]/10)
    {
      cout<<"ERORR:can't find "<<v[i]<<endl;
      break;
    }
    
  }
  finish = clock();
  printf( "\nIt takes %f seconds to self-query %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, v.size());
  
  dd.update(v[1002], v[1002]/3);
  if (*dd.find(v[1002])==v[1002]/3)
    cout<<"Good!";
  
  cout<<dd.del(v[1002])<<endl;
  //cout<<*dd.find(v[1002])<<endl;
  
  if (dd.find(v[1002])==NULL)
    cout<<"Good!";
  
}

BOOST_AUTO_TEST_CASE(dp_hash_string_insertion_check )
{
  vector<string> vstr;
  readDict("./input", vstr);
  
  DynamicPerfectHash<string, int> dp;
  
  clock_t start, finish;
    
  start = clock();
  for (size_t i=0; i<vstr.size(); i++)
  {
    dp.insert(vstr[i], i);
  }
  finish = clock();
  printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, vstr.size());
  
  cout<<dp.save("./data.k", "./data.v")<<" saved!\n";

  //cout<<dp;
  
  DynamicPerfectHash<string, int> dd;
  cout<<dd.load("./data.k", "./data.v")<<" loaded!\n";;
  //cout<<dd;
  start = clock();
  for (size_t i=0; i<vstr.size(); i++)
  {
    if(*dp.find(vstr[i])!= i)
    {
      cout<<"ERORR:can't find "<<vstr[i]<<endl;
      break;
    }
    
  }
  finish = clock();
  printf( "\nIt takes %f seconds to self-query %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, vstr.size());
  
  dd.update(vstr[1002], 99);
  if (*dd.find(vstr[1002])== 99)
    cout<<"Good!";
  
  cout<<dd.del(vstr[1002])<<endl;
  //cout<<*dd.find(v[1002])<<endl;
  
  if (dd.find(vstr[1002])==NULL)
    cout<<"Good!";
  
}

// BOOST_AUTO_TEST_CASE(LHT_insertion_check )
// {

//   vector<string> vstr;
//   readDict("./input", vstr);

//   izenelib::am::LinearHashTable<uint64_t, string> tb;
  
//   clock_t start, finish;
//   start = clock();
//   for (size_t i=0; i<vstr.size(); i++)
//   {
//     tb.insert(vstr[i], 2);
//   }
//   finish = clock();
//   printf( "\nIt takes %f seconds to insert 1000000 random data!\n", (double)(finish - start) / CLOCKS_PER_SEC);
  
//   //  tb.display(std::cout);
   
// }


BOOST_AUTO_TEST_SUITE_END()

