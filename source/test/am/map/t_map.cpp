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
#include <am/map/map.hpp>
#include <am/cccr_hash/cccr_hash.h>
#include <string>
#include <time.h>
#include <math.h>
#include <boost/test/unit_test.hpp>
#include <time.h>
#include <util/log.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <map>
#include <cstdio>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <tr1/unordered_map>
#include <google/dense_hash_map>
using namespace std;

BOOST_AUTO_TEST_SUITE( t_map_suite )

void getRandomSet(int& i, size_t max_len)
{
  i = rand();
}

void getRandomSet(string& str, size_t max_len)
{
  size_t len = 0;
  str= "";
  
  while (len <3)
  {
    len = rand()%max_len;
  }

  while (len > 0)
  {
    str += rand()%26 + 'a';
    len--;
  }
  
}

void getRandomSet(vector<int>& v, size_t max_len)
{
  v.clear();

  size_t len = 0;
  while(len < 3)
  {
    len = rand()%max_len;
  }
  
  while (len > 0)
  {
    v.push_back(rand());
    len--;
  }
  
}

void getRandomSet(map<string, vector<int> >& mp, size_t max_len)
{

  size_t len = 0;
  while(len < 3)
  {
    len = rand()%max_len;
  }

  while (len>0)
  {
    vector<int> v;
    string str;

    getRandomSet(v, max_len);
    getRandomSet(str, max_len);
    mp.insert(pair<string, vector<int> >(str, v));
    len--;
  }
  
}



typedef boost::archive::binary_iarchive iarchive;
typedef boost::archive::binary_oarchive oarchive;

// BOOST_AUTO_TEST_CASE(CCCR_for_numeric_check)
// {
//   #define SIZE 100000
  
//   vector<uint64_t> v;
//   for (int i=0; i<SIZE; i++)
//   {
//     v.push_back(rand());
//   }  
  
//   clock_t start, finish;
//   izenelib::am::Map<uint64_t, uint64_t, 20> tb;
  
//   start = clock();
//   for (size_t i=0; i<v.size(); i++)
//   {
//     tb.insert(v[i], v[i]/10);
//   }
//   finish = clock();
//   printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, v.size());
//   ofstream of("./db");
//   oarchive oa(of);
//   oa << tb;
//   of.close();

  
//   izenelib::am::Map<uint64_t, uint64_t, 20> tb1;
//   ifstream ifs("./db");
//   iarchive ia(ifs);
//   ia >> tb1;
  
//   start = clock();
//   for (size_t i=0; i<v.size(); i++)
//   {
//     if(*tb1.find(v[i])!= v[i]/10)
//     {
//       cout<<"ERORR:can't find "<<v[i]<<endl;
//       break;
//     }
    
//   }
//   finish = clock();
//   printf( "\nIt takes %f seconds to self-query %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, v.size());

//   tb1.update(v[3000], v[3000]/3);
//   if (*tb1.find(v[3000]) == v[3000]/3)
//     cout<<"good!\n";

//   tb1.del(v[3000]);
//   if (tb1.find(v[3000])==NULL)
//     cout<<"good!\n";
// }

template <
  class MAP,
  class KeyType,
  class ValueType
  >
clock_t insert(MAP& map, const KeyType& key, const ValueType& value)
{
  clock_t start, finish;
  start = clock();
  map.insert(pair<KeyType, ValueType>(key, value));
  finish = clock();
  return finish-start;
}

template <
  class MAP,
  class KeyType,
  class ValueType
  >
clock_t find(MAP& map, const KeyType& key)
{
  clock_t start, finish;
  start = clock();
  map.find(key);
  finish = clock();
  return finish-start;
}

BOOST_AUTO_TEST_CASE(izene_map_insert_check)
{

#define PER_TEST_SCALE 500
  #define MAX_OBJ_SCALE 10
  #define TEST_SCALE 1000000
  typedef int KeyType;
  //typedef int ValueType;
  typedef map<string, vector<int> > ValueType;
  
  typedef std::map<KeyType, ValueType> STD_MAP;
  typedef std::tr1::unordered_map<KeyType, ValueType, std::tr1::hash<KeyType> > TR1_MAP;
  typedef google::dense_hash_map<KeyType, ValueType, __gnu_cxx::hash<KeyType> > GG_DENSE_MAP;
  
  
  clock_t izene_insert, std_insert, izene_seri,
    std_seri, izene_find, std_find ,umap_insert, umap_find, umap_seri, ggdense_insert, ggdense_find, ggdense_seri;;
  izene_insert = std_insert = izene_seri = std_seri
    = izene_find = std_find = umap_insert = umap_find = umap_seri = ggdense_insert = ggdense_find =ggdense_seri = 0;

  clock_t start, finish;
  for (int i=0; i<TEST_SCALE; i++)
  {
    if (i%(TEST_SCALE/10)==0)
      cout<<i<<"\n";
    
    izenelib::am::Map<KeyType, ValueType, 9> izene_map;
    STD_MAP std_map;
    TR1_MAP umap;
    GG_DENSE_MAP gg_map;

    gg_map.set_empty_key(0xffffffffu);
    for (int j = 0; j<PER_TEST_SCALE; j++)
    {
      KeyType k;
      ValueType v;
      getRandomSet(k, MAX_OBJ_SCALE);
      getRandomSet(v, MAX_OBJ_SCALE);

      //izene insert
      start = clock();
      izene_map.insert(k, v);
      finish = clock();
      izene_insert += finish - start;

      //std insert
      std_insert += insert<STD_MAP, KeyType, ValueType>(std_map, k, v);
      
      //std::tr1 insert
      umap_insert += insert<TR1_MAP, KeyType, ValueType>(umap, k, v);
            
      //google dense insert
      ggdense_insert += insert<GG_DENSE_MAP, KeyType, ValueType>(gg_map, k, v);

      //izene find
      start = clock();
      izene_map.find(k);
      finish = clock();
      izene_find += finish - start;

      //std find
      std_find += find<STD_MAP, KeyType, ValueType>(std_map, k);
      
      //umap find
      umap_find += find<TR1_MAP, KeyType, ValueType>(umap, k);

      //google find
      ggdense_find += find<GG_DENSE_MAP, KeyType, ValueType>(gg_map, k);
      
    }
    
    //izene seril
    ofstream of1("./db", ios_base::trunc);
    oarchive oa1(of1);
    start = clock();
    oa1<<izene_map;
    finish = clock();
    izene_seri += finish-start;
    of1.close();
    ifstream ifs1("./db");
    iarchive ia1(ifs1);
    start = clock();
    ia1>>izene_map;
    finish = clock();
    izene_seri += finish-start;

    //std seril
    ofstream of("./db", ios_base::trunc);
    oarchive oa(of);
    start = clock();
    oa<<std_map;
    finish = clock();
    std_seri += finish-start;
    of.close();
    ifstream ifs("./db");
    iarchive ia(ifs);
    start = clock();
    ia>>std_map;
    finish = clock();
    std_seri += finish-start;
    
  }  

  cout<<endl;
  cout<<"Map amount: "<<TEST_SCALE<<endl;
  cout<<"Map size: "<<PER_TEST_SCALE<<endl;
  cout<<"Max size of obj in map: "<<MAX_OBJ_SCALE<<endl;
  cout<<"Insert(std/umap/izene): "<<(double)std_insert/izene_insert*100.<<"/"
      <<(double)umap_insert/izene_insert*100.<<"./"
      <<(double)ggdense_insert/izene_insert*100.<<"/100\n";
  cout<<"Find(std/umap/izene): "<<(double)std_find/izene_find*100.<<"/"
      <<(double)umap_find/izene_find*100.<<"/"
      <<(double)ggdense_find/izene_find*100.<<"/100\n";
  cout<<"Serilize&Deserilize(std/izene): "<<(double)std_seri/izene_seri*100.<<"/100\n";
  cout<<"Insert+Find(std/izene): "<<(double)(std_find + std_insert)/(izene_find + izene_insert)*100.<<"/100\n";
  
}


BOOST_AUTO_TEST_SUITE_END()

