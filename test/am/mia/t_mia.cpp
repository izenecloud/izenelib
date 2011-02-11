
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
//#include <util/log.h
#include <am/mia/raw_map.hpp>
#include <am/mia/status.h>
#include <string>
#include <time.h>
#include <math.h>
#include <boost/test/unit_test.hpp>
#include <sys/time.h>
#include <fstream>
#include <iostream>
#include <vector>
#include<stdio.h>

//USING_IZENE_LOG();

BOOST_AUTO_TEST_SUITE( t_MIA_suite )

using namespace izenelib::am;
using namespace std;
using namespace boost::unit_test;

void rand_str(string& s)
{
  s.clear();
  
  size_t l = rand()%10;
  while (l == 0)
    l = rand()%10;

  for (size_t i=0; i<l; ++i)
    s += 'a' + i;
}

void rand_doc(vector<string>& strs, size_t max=100)
{
  strs.clear();
  size_t l = rand()%max;
  while (l == 0)
    l = rand()%max;
  
  for (size_t i=0; i<l; ++i)
  {
    string s;
    rand_str(s);
    strs.push_back(s);
  }
  
}

BOOST_AUTO_TEST_CASE(mining_status_check)
{
  MiningStatus status;
  
  BOOST_CHECK(!status.get_raw_map());
  status.set_raw_map();
  BOOST_CHECK(status.get_raw_map());
  status.reset_raw_map();
  BOOST_CHECK(!status.get_raw_map());
  status.set_raw_map();
  
  BOOST_CHECK(!status.get_forward_index());
  status.set_forward_index();
  BOOST_CHECK(status.get_forward_index());
  status.reset_forward_index();
  BOOST_CHECK(!status.get_forward_index());
  status.set_forward_index();
  
  BOOST_CHECK(!status.get_dupd());
  status.set_dupd();
  BOOST_CHECK(status.get_dupd());
  status.reset_dupd();
  BOOST_CHECK(!status.get_dupd());
  status.set_dupd();
  
  BOOST_CHECK(!status.get_tg());
  status.set_tg();
  BOOST_CHECK(status.get_tg());
  status.reset_tg();
  BOOST_CHECK(!status.get_tg());
  status.set_tg();
  
  BOOST_CHECK(!status.get_qr());
  status.set_qr();
  BOOST_CHECK(status.get_qr());
  status.reset_qr();
  BOOST_CHECK(!status.get_qr());
  status.set_qr();
  
  BOOST_CHECK(!status.get_sim());
  status.set_sim();
  BOOST_CHECK(status.get_sim());
  status.reset_sim();
  BOOST_CHECK(!status.get_sim());
  status.set_sim();

  cout<<"MiningStatus check is done!\n";
}

BOOST_AUTO_TEST_CASE(raw_map_check )
{
  const size_t SIZE= 100000;
  
  typedef RawMap<> Rmp;

  vector<vector<string> > vv;
  for (size_t i = 0; i<SIZE; i++)
  {
    vector<string> v;
    rand_doc(v);
    vv.push_back(v);
  }
  cout<<"Data is ready!\n";

  struct timeval tvafter,tvpre;
  struct timezone tz;

  remove ("./tt.str");
  remove ("./tt.pos");
  Rmp rmp("./tt");
  rmp.ready4Append();

  gettimeofday (&tvpre , &tz);
  for (size_t i = 0; i<SIZE; i++)
  {
    rmp.append(i, vv[i]);
  }
  rmp.flush();
  gettimeofday (&tvafter , &tz);
  cout<<"\nAdd Raw map("<<rmp.doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

  gettimeofday (&tvpre , &tz);
  rmp.ready4FetchByDupD();
  for (size_t i = 0; i<SIZE; i++)
  {
    Rmp::str_vector v;
    BOOST_CHECK( rmp.next4DupD(v)==i);
    vector<string>::iterator k = vv[i].begin();
    for (Rmp::str_vector::const_iterator j=v.begin(); j!=v.end(); ++j, ++k)
    {
      string s(*j);
      BOOST_CHECK(*j==*k);
    }
    
  }
  gettimeofday (&tvafter , &tz);
  cout<<"\nAccess Raw map("<<rmp.doc_num()<<") by DupD: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

  gettimeofday (&tvpre , &tz);
  rmp.ready4FetchByTg();
  for (size_t i = 0; i<SIZE; i++)
  {
    Rmp::str_vector v1;
    Rmp::int_vector v2;
    
    BOOST_CHECK( rmp.next4Tg(v1, v2)==i);
    BOOST_CHECK( v1.length() == v2.size());
    
    vector<string>::iterator k = vv[i].begin();
    for (Rmp::str_vector::const_iterator j=v1.begin(); j!=v1.end(); ++j, ++k)
    {
      string s(*j);
      BOOST_CHECK(*j==*k);
    }

    size_t g = 0;
    for (Rmp::int_vector::const_iterator j=v2.begin(); j!=v2.end(); ++j, ++g)
      BOOST_CHECK(*j == g);
    
  }
  gettimeofday (&tvafter , &tz);
  cout<<"\nAccess Raw map("<<rmp.doc_num()<<") by TG: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

//   gettimeofday (&tvpre , &tz);
//   rmp.ready4FetchByTg();
//   for (size_t i = 0; i<SIZE; i++)
//   {
//     Rmp::str_vector v1;
//     Rmp::int_vector v2;

//     rmp.get4Tg(i, v1, v2);
    
//     BOOST_CHECK( v1.length() == v2.size());
    
//     vector<string>::iterator k = vv[i].begin();
//     for (Rmp::str_vector::const_iterator j=v1.begin(); j!=v1.end(); ++j, ++k)
//     {
//       string s(*j);
//       BOOST_CHECK(*j==*k);
//     }

//     size_t g = 0;
//     for (Rmp::int_vector::const_iterator j=v2.begin(); j!=v2.end(); ++j, ++g)
//       BOOST_CHECK(*j == g);
    
//   }
//   gettimeofday (&tvafter , &tz);
//   cout<<"\nSearch in Raw map("<<rmp.doc_num()<<") by TG: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
  
}

BOOST_AUTO_TEST_CASE(raw_map_performance_check )
{
  const size_t SIZE= 50000000;
  
  typedef RawMap<1000, 1000> Rmp;

  
  struct timeval tvafter,tvpre;
  struct timezone tz;
  remove ("./tt.str");
  remove ("./tt.pos");
  Rmp rmp("./tt");
  rmp.ready4Append();

  gettimeofday (&tvpre , &tz);
  for (size_t i = 0; i<SIZE; i++)
  {
    vector<string> v;
    rand_doc(v);
    rmp.append(i, v);
  }
  rmp.flush();
  gettimeofday (&tvafter , &tz);
  cout<<"\nAdd Raw map("<<rmp.doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

  gettimeofday (&tvpre , &tz);
  rmp.ready4FetchByDupD();
  for (size_t i = 0; i<SIZE; i++)
  {
    Rmp::str_vector v;
    BOOST_CHECK( rmp.next4DupD(v)==i);
  }
  gettimeofday (&tvafter , &tz);
  cout<<"\nAccess Raw map("<<rmp.doc_num()<<") by DupD: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

  gettimeofday (&tvpre , &tz);
  rmp.ready4FetchByTg();
  for (size_t i = 0; i<SIZE; i++)
  {
    Rmp::str_vector v1;
    Rmp::int_vector v2;
    
    BOOST_CHECK( rmp.next4Tg(v1, v2)==i);    
  }
  gettimeofday (&tvafter , &tz);
  cout<<"\nAccess Raw map("<<rmp.doc_num()<<") by TG: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
  
}

BOOST_AUTO_TEST_SUITE_END()
