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
#include <am/str_dynamic_array/str_dynamic_array.hpp>

#include <string>
#include <time.h>
#include <math.h>
#include <boost/test/unit_test.hpp>
#include <time.h>

#include <fstream>
#include <iostream>
#include <math.h>

#include <cstdio>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <algorithm>

using namespace std;

typedef boost::archive::binary_iarchive iarchive;
typedef boost::archive::binary_oarchive oarchive;

BOOST_AUTO_TEST_SUITE( t_dyn_str_array_suite )

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



typedef boost::archive::binary_iarchive iarchive;
typedef boost::archive::binary_oarchive oarchive;


BOOST_AUTO_TEST_CASE(dyn_insertion_check )
{  
  vector<string> vstr;
  readDict("./input", vstr);
  
  DynamicStrArray dsa;
  vector<string> vs;
  
  clock_t start, finish, vs_t, dsa_t = 0;
  vs_t = dsa_t = 0;

  start = clock();
  for (size_t i=0; i<vstr.size(); i++)
    dsa.push_back(vstr[i]);
  finish = clock();
  dsa_t += finish -start ;

  start = clock();
  for (size_t i=0; i<vstr.size(); i++)
    vs.push_back(vstr[i]);
  finish = clock();
  vs_t += finish -start ;
    
  printf( "\nIt takes %f seconds for dynamic string array to insert %d random strings\n", (double)dsa_t/ CLOCKS_PER_SEC, vstr.size());
  printf( "\nIt takes %f seconds for vector<string> to insert %d random strings\n", (double)vs_t/ CLOCKS_PER_SEC, vstr.size());


  dsa_t = vs_t = 0;
  ofstream of1("./db", ios_base::trunc);
  oarchive oa1(of1);
  start = clock();
  oa1<<vs;
  finish = clock();
  vs_t += finish-start;
  of1.close();
  ifstream ifs1("./db");
  iarchive ia1(ifs1);
  start = clock();
  ia1>>vs;
  finish = clock();
  vs_t += finish-start;

  ofstream of("./db", ios_base::trunc);
  oarchive oa(of);
  start = clock();
  oa<<dsa;
  finish = clock();
  dsa_t += finish-start;
  of.close();
  ifstream ifs("./db");
  iarchive ia(ifs);
  start = clock();
  ia>>dsa;
  finish = clock();
  dsa_t += finish-start;

  printf( "\nIt takes %f seconds for dynamic string array to serialize & deserialize %d random strings\n", (double)dsa_t/ CLOCKS_PER_SEC, vstr.size());
  printf( "\nIt takes %f seconds for vector<string> to serialize & deserialize %d random strings\n", (double)vs_t/ CLOCKS_PER_SEC, vstr.size());

  dsa_t = vs_t = 0;
  start = clock();
  dsa.sort();
  finish = clock();
  dsa_t += finish-start;
  printf( "\nIt takes %f seconds for dynamic string array to sort\n", (double)dsa_t/ CLOCKS_PER_SEC);

  start = clock();
  sort(vs.begin(),vs.end());
  finish = clock();
  vs_t += finish-start;
  printf( "\nIt takes %f seconds for vector<string> to sort\n", (double)vs_t/ CLOCKS_PER_SEC);

  DynamicStrArray::const_iterator it =dsa.begin();
  it++;
  it++;  
  dsa_t = vs_t = 0;
  start = clock();
  for (size_t i=0; i<vstr.size()/100000; i++)
    dsa.insert(it, vstr[i]);
  finish = clock();
  dsa_t += finish -start ;
  printf( "\nIt takes %f seconds for DynammicStrArray to insert %d strings\n", (double)dsa_t/ CLOCKS_PER_SEC, vstr.size()/100000);

  vector<string>::iterator itt =vs.begin();
  itt++;
  itt++;
  start = clock();
  for (size_t i=0; i<vstr.size()/100000; i++)
    vs.insert(itt, vstr[i]);
  finish = clock();
  vs_t += finish -start ;
  printf( "\nIt takes %f seconds for vector<string> to insert %d strings\n", (double)vs_t/ CLOCKS_PER_SEC, vstr.size()/100000);

  
  dsa_t = vs_t = 0;
  start = clock();
  for (DynamicStrArray::const_iterator i =dsa.begin(); i!=dsa.end();i++)
  {
    string s = *i;
    cout<<s<<endl;
  }
  finish = clock();
  dsa_t += finish -start ;
  printf( "\nIt takes %f seconds for DynammicStrArray's iterator to get %d strings\n", (double)dsa_t/ CLOCKS_PER_SEC, vstr.size());

  start = clock();
  for (vector<string>::const_iterator i =vs.begin(); i!=vs.end();i++)
    string s = *i;
  finish = clock();
  vs_t += finish -start ;
  printf( "\nIt takes %f seconds for vector<string>'s iterator to get %d strings\n", (double)vs_t/ CLOCKS_PER_SEC, vstr.size());

  dsa_t = vs_t = 0;
  start = clock();
  for (size_t i=0; i<dsa.size(); i++)
    string s = dsa[i];
  finish = clock();
  dsa_t += finish -start ;
  printf( "\nIt takes %f seconds for dynamic string array's operator [] to get %d strings\n", (double)dsa_t/ CLOCKS_PER_SEC, vstr.size());

  start = clock();
  for (size_t i=0; i<vs.size(); i++)
    string ss = vs[i];
  finish = clock();
  vs_t += finish -start ;
  printf( "\nIt takes %f seconds for vector<string>'s operator [] to get %d strings\n", (double)vs_t/ CLOCKS_PER_SEC, vstr.size());  
}

BOOST_AUTO_TEST_SUITE_END()

