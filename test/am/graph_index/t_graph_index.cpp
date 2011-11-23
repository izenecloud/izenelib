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

#include <am/graph_index/dyn_array.hpp>
#include <am/graph_index/integer_hash.hpp>
#include <am/graph_index/id_transfer.hpp>
#include <am/graph_index/sorter.hpp>
#include <am/graph_index/graph.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <time.h>
#include <math.h>

#include <sys/time.h>
#include <fstream>
#include <iostream>
#include <vector>
#include<stdio.h>

using namespace izenelib::am;
using namespace std;

uint32_t error_count = 0;

BOOST_AUTO_TEST_SUITE(graph_index_test)

#define CHECK_(f)\
  {                                                \
    if (!(f)){ BOOST_CHECK(false);++error_count; std::cout<<"ERROR: "<<__FILE__<<": "<<__LINE__<<": "<<__FUNCTION__<<endl;} \
  }
#define ERROR_COUNT {if(error_count>0)cout<<endl<<error_count<<" errors ware found!";else{cout<<"\nNo error detected!\n"}}

struct TEST_STRUCT
{
  char integer[4];
  char one;

  uint32_t& INTEGER_()
  {
    return *(uint32_t*)integer;
  }

  char& ONE_()
  {
    return one;
  }

  uint32_t INTEGER()const
  {
    return *(uint32_t*)integer;
  }

  char ONE()const
  {
    return one;
  }

  inline TEST_STRUCT(uint32_t i, char j)
  {
    INTEGER_() = i;
    ONE_() = j;
  }

  inline TEST_STRUCT(uint32_t i)
  {
    INTEGER_() = i;
    ONE_() = 0;
  }

  inline TEST_STRUCT()
  {
    INTEGER_() = 0;
    ONE_() = 0;
  }

  inline TEST_STRUCT(const TEST_STRUCT& other)
  {
    INTEGER_() = other.INTEGER();
    ONE_() = other.ONE();
  }

  inline TEST_STRUCT& operator = (const TEST_STRUCT& other)
  {
    INTEGER_() = other.INTEGER();
    ONE_() = other.ONE();
    return *this;
  }

  inline bool operator == (const TEST_STRUCT& other)const
  {
    return (INTEGER() == other.INTEGER());
  }

  inline bool operator != (const TEST_STRUCT& other)const
  {
    return (INTEGER() != other.INTEGER());
  }

  inline bool operator < (const TEST_STRUCT& other)const
  {
    return (INTEGER() < other.INTEGER());
  }

  inline bool operator > (const TEST_STRUCT& other)const
  {
    return (INTEGER() > other.INTEGER());
  }

  inline bool operator <= (const TEST_STRUCT& other)const
  {
    return (INTEGER() <= other.INTEGER());
  }

  inline bool operator >= (const TEST_STRUCT& other)const
  {
    return (INTEGER() >= other.INTEGER());
  }

  inline uint32_t operator % (uint32_t e)const
  {
    return (INTEGER() % e);
  }



}
  ;

template<typename VALUE_TYPE>
VALUE_TYPE random()
{
  return rand();
}

template<>
struct TEST_STRUCT random<struct TEST_STRUCT>()
{
  return TEST_STRUCT(rand(), 1);
}

template<typename VALUE_TYPE>
void dyn_array_check(const VALUE_TYPE& t = VALUE_TYPE())
{
  cout<<"DynArray checking....\n";

  boost::filesystem::remove_all("./tt");
  vector<VALUE_TYPE> v;
  typedef DynArray<VALUE_TYPE> Array;
  {
    const size_t SIZE=500000;

    for (size_t i=0; i<SIZE; i++)
      v.push_back(random<VALUE_TYPE>());

    Array ar(v);
    Array br = ar;

    CHECK_(ar == br);
    br[1000] = VALUE_TYPE(222);
    v[1000] = VALUE_TYPE(222);
    CHECK_(br == v);


    clock_t start, vt, at;
    vt = at = 0;

    for (size_t i=0; i<SIZE; i++)
    {
      VALUE_TYPE k = random<VALUE_TYPE>();

      start = clock();
      br.push_back(k);
      at += clock()-start;

      start = clock();
      v.push_back(k);
      vt += clock()-start;

    }

    printf( "\n[push_back] vector: %f My Array: %f !\n", (double)(vt) / CLOCKS_PER_SEC, (double)(at) / CLOCKS_PER_SEC);

    br.compact();
    CHECK_(br == v);
    CHECK_(br.find(ar[100])==100);
    CHECK_(br.find(ar[1003])==1003);

    br = ar;


    br += v;

    Array cr(v);
    ar += cr;
    CHECK_(br == ar);


    //----------------------------------------------

    typedef DynArray<VALUE_TYPE, true> SortedArray;
    SortedArray sa;
    for (size_t i=0; i<SIZE/1000; i++)
    {
      sa.push_back(random<VALUE_TYPE>());
      //cout<<sa<<endl;
    }

    for (size_t i=0; i<sa.length()-1; i++)
      CHECK_(sa[i]<=sa[i+1]);

    CHECK_(sa.find(sa[10])==10);
    CHECK_(sa.find(sa[5])==5);

    cr = v;
    FILE* f = fopen("./tt", "w+");
    CHECK_(cr.save(f) == cr.save_size());
    fclose(f);
  }

  {
    Array array;
    array = v;

    FILE* f = fopen("./tt", "r");
    array.load(f);
    fclose(f);

    CHECK_(array == v);

    f = fopen("./tt", "w+");
    array.compressed_save(f);
    fclose(f);
  }

  {
    Array array;
    array = v;

    FILE* f = fopen("./tt", "r");
    array.compressed_load(f);
    fclose(f);

    CHECK_(array == v);
  }

  {
    Array array;

    const size_t SIZE=5000;//0000;

    for (size_t i=0; i<SIZE; i++)
      array.push_back(random<VALUE_TYPE>());

    clock_t start, finish;
    start = clock();
    array.sort();
    finish = clock();
    printf( "\n[sort] Array: %f !\n", (double)(finish-start) / CLOCKS_PER_SEC);

    for (size_t i=0; i<SIZE-1; i++)
      CHECK_(array.at(i)<=array.at(i+1));
  }

}

template<typename VALUE_TYPE>
void integer_hash_check(const VALUE_TYPE& t = VALUE_TYPE())
{
  cout<<"IntegerHashTable checking....\n";

  boost::filesystem::remove_all("./tt");

  vector<VALUE_TYPE> v;
  typedef IntegerHashTable<VALUE_TYPE> hash_t;

  {
    const uint32_t SIZE=1000000;

    for (size_t i=0; i<SIZE; i++)
      v.push_back(random<VALUE_TYPE>());

    hash_t hash;
    clock_t start, finish;
    start = clock();
    for (size_t i=0; i<v.size(); ++i)
      hash.insert(v[i]);
    finish = clock();
    printf( "\nIntegerHashTable insert(%d): %f s!\n", SIZE, (double)(finish-start) / CLOCKS_PER_SEC);

    start = clock();
    for (size_t i=0; i<v.size(); ++i)
    {
      VALUE_TYPE t = v[i];
      CHECK_(hash.find(t));
    }
    finish = clock();
    printf( "\nIntegerHashTable find(%d): %f s!\n", SIZE, (double)(finish-start) / CLOCKS_PER_SEC);

    FILE* f = fopen("./tt", "w+");
    CHECK_(hash.save(f)==hash.save_size());
    fclose(f);
  }
  {
    hash_t hash;
    //clock_t start, finish;

    FILE* f = fopen("./tt", "r");
    hash.load(f);

    fclose(f);

    for (size_t i=0; i<v.size(); ++i)
    {
      VALUE_TYPE t = v[i];
      CHECK_(hash.find(t));
    }
  }

}

void id_transfer_check()
{
  cout<<"IDtransfer checking ...\n";

  boost::filesystem::remove_all("./tt");
  typedef IdTransfer<> id_t;

  const uint32_t SIZE=1000000;

  {
    id_t id_tansfer;

    clock_t start, finish;
    start = clock();
    for (size_t i=0; i<SIZE; i++)
      CHECK_(id_tansfer.insert(SIZE+i) == i+1);
    finish = clock();
    printf( "\nIdTransfer insert(%d): %f s!\n", SIZE, (double)(finish-start) / CLOCKS_PER_SEC);

    start = clock();
    for (size_t i=0; i<SIZE; i++)
      CHECK_(id_tansfer.get32(SIZE+i)==i+1);
    finish = clock();
    printf( "\nIdTransfer find(%d): %f s!\n", SIZE, (double)(finish-start) / CLOCKS_PER_SEC);

    for (size_t i=0; i<SIZE; i++)
      CHECK_(id_tansfer.get64(i+1)==i+SIZE);

    FILE* f = fopen("./tt", "w+");
    id_tansfer.save(f);
    fclose(f);
  }
  {
    id_t id_tansfer;

    FILE* f = fopen("./tt", "r");

    id_tansfer.load(f);
    fclose(f);

    for (size_t i=0; i<SIZE; i++)
      CHECK_(id_tansfer.get32(SIZE+i)==i+1);

    for (size_t i=0; i<SIZE; i++)
      CHECK_(id_tansfer.get64(i+1)==i+SIZE);

    for (size_t i=0; i<SIZE; i++)
      CHECK_(id_tansfer.insert(SIZE+i) == i+1);


    for (size_t i=0; i<SIZE; i++)
      CHECK_(id_tansfer.get32(SIZE+i)==i+1);

    for (size_t i=0; i<SIZE; i++)
      CHECK_(id_tansfer.get64(i+1)==i+SIZE);

  }

}

void sorter_check()
{
  typedef DynArray<uint32_t> terms_t;
  {
    boost::filesystem::remove_all("./tt");

    struct timeval tvafter,tvpre;
    struct timezone tz;

    const uint32_t SIZE = 1000000;
    const uint32_t snip_len = 10;
    vector<uint32_t> vs;

    Sorter<
    > sorter("./tt");

    terms_t terms;

    sorter.ready4add();

    gettimeofday (&tvpre , &tz);
    for (uint32_t p = 0; p<10; ++p)
    {
      vs.resize(SIZE);
      for (uint64_t i=0; i<SIZE; ++i)
      {
        vs[i] = (rand()%80000);
      }

      uint32_t docid = 0;
      for (size_t i=0; i<vs.size()-snip_len; i+=snip_len)
      {
        vector<uint32_t> terms;
        for (size_t j=i; j<i+snip_len; ++j)
        {
          //cout<<vs[j]<<" ";
          terms.push_back(vs[j]);
        }
        //cout<<endl;

        if (i%(10*snip_len)==0)
          ++docid;

        sorter.add_terms(terms_t(terms), docid);
      }
    }
    gettimeofday (&tvafter , &tz);
    cout<<"\nInsert into sorter ("<<sorter.num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";

    gettimeofday (&tvpre , &tz);
    sorter.flush();
    sorter.sort();
    gettimeofday (&tvafter , &tz);
    cout<<"\nSorter to sort ("<<sorter.num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";

    sorter.ready4fetch();
    uint32_t docid = 0;
    terms_t last;
    terms_t cur;
    sorter.next(last, docid);
    uint32_t addr = last.size()+sizeof(uint16_t)+sizeof(uint32_t);
    for (uint32_t i=1; i<sorter.num(); ++i)
    {
      sorter.next(cur, docid);
      addr += cur.size()+sizeof(uint16_t)+sizeof(uint32_t);

//       std::cout<<i<<": "<<addr<<"---\n";
//       cout<<last<<"--"<<cur<<endl;

      CHECK_(last[0]<=cur[0]);

      if(last[0]>cur[0])
      {
        cout<<"ERROR: "<<last<<"++"<<cur<<endl;
        break;
      }

      last = cur;
    }

  }

}

void construct_trie(char* name, uint32_t num)
{
  Graph<> graph(name);

  struct timeval tvafter,tvpre;
  struct timezone tz;

  const uint32_t SIZE = num;
  const uint32_t snip_len = 10;
  vector<uint64_t> vs;

  graph.ready4add();

  gettimeofday (&tvpre , &tz);
  for (uint32_t p = 0; p<10; ++p)
  {
    vs.resize(SIZE);
    for (uint64_t i=0; i<SIZE; ++i)
      vs[i] = (rand()%80000);

    uint32_t docid = 0;
    for (size_t i=0; i<vs.size()-snip_len; i+=snip_len)
    {
      vector<uint32_t> terms;
      for (size_t j=i; j<i+snip_len; ++j)
      {
        //cout<<vs[j]<<" ";
        terms.push_back(vs[j]);
      }
      //cout<<endl;

      if (i%(10*snip_len)==0)
        ++docid;

      graph.add_terms(terms, docid);
    }
  }
  gettimeofday (&tvafter , &tz);
  cout<<"\nInsert into graph ("<<graph.doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";

  gettimeofday (&tvpre , &tz);
  graph.indexing();
  gettimeofday (&tvafter , &tz);
  cout<<"\nGraph indexing: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";

  //graph.ratio_load();
}

void graph_check()
{
  cout<<"Graph checking ...\n";

  boost::filesystem::remove_all("./tt");
  {
    Graph<> graph("./tt");
    typedef DynArray<uint32_t> Array;

    const uint32_t SIZE = 50000;
    vector<vector<uint32_t> > vs;

    graph.ready4add();

    for (uint32_t p = 0; p<SIZE; ++p)
    {
      uint32_t len = rand()%15;

      while (len<=2)len = rand()%15;
      vector<uint32_t> terms;
      for (size_t j=0; j<len; ++j)
        terms.push_back(rand()%80000);

      vs.push_back(terms);

      graph.add_terms(terms, 0);
    }

    graph.indexing();

    graph.ratio_load(.88);

    for (uint32_t p = 0; p<SIZE; ++p)
      if (graph.get_freq(vs[p])<1)
      {
        Array arr(vs[p]);
        cout<<"[ERROR]: "<<arr<<endl;
      }
  }

  boost::filesystem::remove_all("./tt");

}

BOOST_AUTO_TEST_CASE(dyn_array_test)
{
   dyn_array_check<uint64_t>();
   dyn_array_check<struct TEST_STRUCT>();
}

BOOST_AUTO_TEST_CASE(integer_hash_test)
{
  integer_hash_check<uint64_t>();
  integer_hash_check<struct TEST_STRUCT>();
}

BOOST_AUTO_TEST_CASE(id_transfer_test)
{
  id_transfer_check();
}

BOOST_AUTO_TEST_CASE(sorter_test)
{
  sorter_check();
}

BOOST_AUTO_TEST_CASE(graph_test)
{
  graph_check();
}

BOOST_AUTO_TEST_SUITE_END()
