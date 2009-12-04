
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

#define CHECK(f)\
  {                                                \
    if (!(f)){ ++error_count; std::cout<<"ERROR: "<<__FILE__<<": "<<__LINE__<<": "<<__FUNCTION__<<endl;} \
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

  system("rm -fr ./tt*");
  vector<VALUE_TYPE> v;
  typedef DynArray<VALUE_TYPE> Array;
  {
    const size_t SIZE=500000;
    
    for (size_t i=0; i<SIZE; i++)
      v.push_back(random<VALUE_TYPE>());

    Array ar(v);
    Array br = ar;

    CHECK(ar == br);
    br[1000] = VALUE_TYPE(222);
    v[1000] = VALUE_TYPE(222);
    CHECK(br == v);
  
   
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
    CHECK(br == v);
    CHECK(br.find(ar[100])==100);
    CHECK(br.find(ar[1003])==1003);

    br = ar;

  
    br += v;
  
    Array cr(v);
    ar += cr;
    CHECK(br == ar);
  

    //----------------------------------------------

    typedef DynArray<VALUE_TYPE, true> SortedArray;
    SortedArray sa;
    for (size_t i=0; i<SIZE/1000; i++)
    {
      sa.push_back(random<VALUE_TYPE>());
      //cout<<sa<<endl;
    }

    for (size_t i=0; i<sa.length()-1; i++)
      CHECK(sa[i]<=sa[i+1]);
  
    CHECK(sa.find(sa[10])==10);
    CHECK(sa.find(sa[5])==5);

    cr = v;
    FILE* f = fopen("./tt", "w+");
    CHECK(cr.save(f) == cr.save_size());
    fclose(f);
  }

  {
    Array array;
    array = v;

    FILE* f = fopen("./tt", "r");
    array.load(f);
    fclose(f);

    CHECK(array == v);
  }  

  {
    Array array;

    const size_t SIZE=50000000;
    
    for (size_t i=0; i<SIZE; i++)
      array.push_back(random<VALUE_TYPE>());

    clock_t start, finish;
    start = clock();
    array.merge_sort();
    finish = clock();
    printf( "\n[sort] Array: %f !\n", (double)(finish-start) / CLOCKS_PER_SEC);

    for (size_t i=0; i<SIZE-1; i++)
      CHECK(array.at(i)<=array.at(i+1));
  }
  
}

template<typename VALUE_TYPE>
void integer_hash_check(const VALUE_TYPE& t = VALUE_TYPE())
{
  cout<<"IntegerHashTable checking....\n";

  system("rm -fr ./tt*");
  
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
      CHECK(hash.find(t));
    }
    finish = clock();
    printf( "\nIntegerHashTable find(%d): %f s!\n", SIZE, (double)(finish-start) / CLOCKS_PER_SEC);

    FILE* f = fopen("./tt", "w+");
    CHECK(hash.save(f)==hash.save_size());
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
      CHECK(hash.find(t));
    }
  }

}

void id_transfer_check()
{
  cout<<"IDtransfer checking ...\n";

  system("rm -fr ./tt*");
  typedef IdTransfer<> id_t;

  const uint32_t SIZE=1000000;
  
  {
    id_t id_tansfer;

    clock_t start, finish;
    start = clock();
    for (size_t i=0; i<SIZE; i++)
      CHECK(id_tansfer.insert(SIZE+i) == i+1);
    finish = clock();
    printf( "\nIdTransfer insert(%d): %f s!\n", SIZE, (double)(finish-start) / CLOCKS_PER_SEC);

    start = clock();
    for (size_t i=0; i<SIZE; i++)
      CHECK(id_tansfer.get32(SIZE+i)==i+1);
    finish = clock();
    printf( "\nIdTransfer find(%d): %f s!\n", SIZE, (double)(finish-start) / CLOCKS_PER_SEC);

    for (size_t i=0; i<SIZE; i++)
      CHECK(id_tansfer.get64(i+1)==i+SIZE);

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
      CHECK(id_tansfer.get32(SIZE+i)==i+1);

    for (size_t i=0; i<SIZE; i++)
      CHECK(id_tansfer.get64(i+1)==i+SIZE);

    for (size_t i=0; i<SIZE; i++)
      CHECK(id_tansfer.insert(SIZE+i) == i+1);

    
    for (size_t i=0; i<SIZE; i++)
      CHECK(id_tansfer.get32(SIZE+i)==i+1);

    for (size_t i=0; i<SIZE; i++)
      CHECK(id_tansfer.get64(i+1)==i+SIZE);

  }
  
}

void sorter_check()
{
  typedef DynArray<uint32_t> terms_t;
  {
    system("rm -fr ./tt*");

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
      
      CHECK(last[0]<=cur[0]);
      
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

  system("rm -fr ./tt*");

  {
     
    vector<uint32_t> vs;


    Graph<> graph("./tt");

    graph.ready4add();
    
    vs.push_back(7);vs.push_back(2);vs.push_back(8);vs.push_back(9);vs.push_back(10);
    vs.push_back(11);vs.push_back(12);vs.push_back(13);
    graph.add_terms(vs, 1);
    vs.clear();
  
    vs.push_back(14);vs.push_back(15);vs.push_back(13);
    graph.add_terms(vs, 1);
    vs.clear();
  
    vs.push_back(1);vs.push_back(2);vs.push_back(3);vs.push_back(4);
    graph.add_terms(vs, 2);
    vs.clear();
  
    vs.push_back(5);vs.push_back(2);vs.push_back(3);vs.push_back(4);vs.push_back(6);
    graph.add_terms(vs, 3);
    vs.clear();

    vs.push_back(3);vs.push_back(5);vs.push_back(3);vs.push_back(7);vs.push_back(6);
    graph.add_terms(vs, 3);
    vs.clear();

    vs.push_back(5);vs.push_back(7);vs.push_back(7);vs.push_back(5);vs.push_back(6);
    graph.add_terms(vs, 4);
    vs.clear();

    vs.push_back(15);vs.push_back(17);vs.push_back(17);vs.push_back(15);vs.push_back(13);
    graph.add_terms(vs, 5);
    vs.clear();

    CHECK(graph.doc_num() == 5 );

    //cout<<"\nStart indexing.............\n";
    graph.indexing();
    graph.ratio_load(0.2);

    //test for appending
    graph.ready4update();
    
    vs.push_back(105);vs.push_back(107);vs.push_back(107);vs.push_back(105);vs.push_back(103);
    graph.append_terms(vs, 6);
    vs.clear();

    vs.push_back(15);vs.push_back(17);vs.push_back(5);vs.push_back(15);vs.push_back(17);
    graph.append_terms(vs, 8);
    vs.clear();

    vs.push_back(17);vs.push_back(4);vs.push_back(5);vs.push_back(2);vs.push_back(3);
    graph.append_terms(vs, 8);
    vs.clear();

    //test delete terms
    vs.push_back(9);vs.push_back(7);vs.push_back(107);vs.push_back(105);vs.push_back(103);
    graph.append_terms(vs, 9);
    vs.clear();

    vs.push_back(15);vs.push_back(11);vs.push_back(5);vs.push_back(15);vs.push_back(7);
    graph.append_terms(vs, 10);
    vs.clear();

    vs.push_back(8);vs.push_back(4);vs.push_back(5);vs.push_back(2);vs.push_back(3);
    graph.append_terms(vs, 10);
    vs.clear();

    vs.push_back(9);vs.push_back(7);vs.push_back(107);vs.push_back(105);vs.push_back(103);
    graph.del_terms(vs, 9);
    vs.clear();

    vs.push_back(15);vs.push_back(11);vs.push_back(5);vs.push_back(15);vs.push_back(7);
    graph.del_terms(vs, 10);
    vs.clear();

    vs.push_back(8);vs.push_back(4);vs.push_back(5);vs.push_back(2);vs.push_back(3);
    graph.del_terms(vs, 10);
    vs.clear();

    graph.flush();

    //graph.compact();
    
    graph.ratio_load(0.2);
    //std::cout<<graph;
    
    vs.push_back(7);
    //std::cout<<graph.get_freq(vs)<<" MMMMMMMMMM\n";
    CHECK(graph.get_freq(vs)==4);
    vs.clear();

    vs.push_back(2);
    //std::cout<<graph.get_freq(vs)<<" MMMMMMMMMM\n";
    CHECK(graph.get_freq(vs)== 4);
    vs.clear();

    vs.push_back(5);
    //std::cout<<graph.get_freq(vs)<<" MMMMMMMMMM\n";
    CHECK(graph.get_freq(vs)==6);
    vs.clear();

    vs.push_back(2);
    vs.push_back(3);
    //std::cout<<graph.get_freq(vs)<<" MMMMMMMMMM\n";
    CHECK(graph.get_freq(vs)==3);
    vs.clear();

    vs.push_back(5);vs.push_back(2);vs.push_back(3);vs.push_back(4);
    vs.push_back(6);
    CHECK(graph.get_freq(vs)==1);
    vs.clear();

    vector<uint32_t> suffix;
    vector<uint32_t> counts;
    vs.push_back(15);
    graph.get_suffix(vs, suffix, counts);
    CHECK(suffix.size()==2);
    CHECK(counts.size()==2);
    for (size_t i=0; i<suffix.size();++i)
    {
      if (suffix[i] == 13)
      {
        CHECK(counts[i]==2);
      }    
      else if(suffix[i]==17)
      {
        CHECK(counts[i]==3);
      }
      else
        CHECK(false);
    }
    vs.clear();

    vs.push_back(2);vs.push_back(3);
    graph.get_suffix(vs, suffix, counts);
    CHECK(suffix.size()==1);
    CHECK(counts.size()==1);
    for (size_t i=0; i<suffix.size();++i)
    {
      if (suffix[i]==4)
      {
        CHECK(counts[i]==2);
      }
      else
        CHECK(false);
    }
    vs.clear();

    graph.get_doc_list(suffix, suffix);
  }

  system("rm -fr ./tt*");
  {
    ofstream of("./of");
    Graph<> graph("./tt");
    typedef DynArray<uint32_t> Array;
    
    struct timeval tvafter,tvpre;
    struct timezone tz;
  
    const uint32_t SIZE = 5000;
    const uint32_t snip_len = 10;
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

  
  system("rm -fr ./tt*");
  construct_trie("./tt", 100000);
  
  {
    Graph<> graph("./tt");
    
    struct timeval tvafter,tvpre;
    struct timezone tz;
  
    const uint32_t SIZE = 100000;
    const uint32_t snip_len = 10;
    vector<uint64_t> vs;
    
    graph.ready4update();

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

        graph.append_terms(terms, docid);
      }
    }

    graph.flush();
    gettimeofday (&tvafter , &tz);
    cout<<"\nAppend into graph ("<<graph.doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";
    //graph.ratio_load();
  }
  
}


void graph_merge_check()
{
  cout<<"Graph checking ...\n";

  system("rm -fr ./tt*");

  vector<uint32_t> vs;

  {
      
    Graph<> graph("./tt1");

    graph.ready4add();
    
    vs.push_back(7);vs.push_back(2);vs.push_back(8);vs.push_back(9);vs.push_back(10);
    vs.push_back(11);vs.push_back(12);vs.push_back(13);
    graph.add_terms(vs, 1);
    vs.clear();
  
    vs.push_back(14);vs.push_back(15);vs.push_back(13);
    graph.add_terms(vs, 1);
    vs.clear();
  
    vs.push_back(1);vs.push_back(2);vs.push_back(3);vs.push_back(4);
    graph.add_terms(vs, 2);
    vs.clear();
  
    vs.push_back(5);vs.push_back(2);vs.push_back(3);vs.push_back(4);vs.push_back(6);
    graph.add_terms(vs, 3);
    vs.clear();

    vs.push_back(3);vs.push_back(5);vs.push_back(3);vs.push_back(7);vs.push_back(6);
    graph.add_terms(vs, 3);
    vs.clear();

    vs.push_back(5);vs.push_back(7);vs.push_back(7);vs.push_back(5);vs.push_back(6);
    graph.add_terms(vs, 4);
    vs.clear();

    vs.push_back(15);vs.push_back(17);vs.push_back(17);vs.push_back(15);vs.push_back(13);
    graph.add_terms(vs, 5);
    vs.clear();

    CHECK(graph.doc_num() == 5 );

    //cout<<"\nStart indexing.............\n";
    graph.indexing();
  }
  {
    Graph<> graph("./tt2");

    //test for appending
    graph.ready4add();
    
    vs.push_back(105);vs.push_back(107);vs.push_back(107);vs.push_back(105);vs.push_back(103);
    graph.add_terms(vs, 6);
    vs.clear();

    vs.push_back(15);vs.push_back(17);vs.push_back(5);vs.push_back(15);vs.push_back(17);
    graph.add_terms(vs, 8);
    vs.clear();

    vs.push_back(17);vs.push_back(4);vs.push_back(5);vs.push_back(2);vs.push_back(3);
    graph.add_terms(vs, 8);
    vs.clear();
    
    graph.indexing();
  }
  {
    Graph<> graph("./tt3");

    //test for appending
    graph.ready4add();
    
    vs.push_back(1115);vs.push_back(1077);vs.push_back(1077);vs.push_back(1055);vs.push_back(1033);
    graph.add_terms(vs, 45);
    vs.clear();

    vs.push_back(155);vs.push_back(177);vs.push_back(55);vs.push_back(15);vs.push_back(127);
    graph.add_terms(vs, 99);
    vs.clear();

    vs.push_back(177);vs.push_back(44);vs.push_back(55);vs.push_back(22);vs.push_back(33);
    graph.add_terms(vs, 99);
    vs.clear();
    
    graph.indexing();
  }
  {
    Graph<> graph("./tt1");
    Graph<> graph2("./tt2");
    Graph<> graph3("./tt3");

    // graph.ratio_load(0.2);
//     std::cout<<graph<<std::endl;
//     cout<<"-----------------\n";
//     graph2.ratio_load(0.2);
//     std::cout<<graph2<<std::endl;

    std::cout<<"Start merging...\n";
    graph.merge(graph2);
    
    {
      graph2.ratio_load();
      graph.ratio_load();
      // cout<<graph<<endl;
//       cout<<"===========\n";
//       cout<<graph2<<endl;
      Graph<>::Node root1 = graph.get_root();
      Graph<>::Node root2 = graph2.get_root();

      Graph<>::NodeIterator it = root2.children_begin();
      while(it != root2.children_end())
      {
        Graph<>::Node tmp;
        CHECK(graph2.get_node(root1, (*it).get_term(), tmp));
        ++it;
      }
    }
    
    
    graph.merge(graph3);
  

    std::cout<<"Start counter-merging...\n";
    graph.counter_merge(graph3);
  }
  {
    Graph<> graph("./tt1");
    graph.ratio_load(0.2);
    //std::cout<<graph<<std::endl;
    
    vs.push_back(7);
    //std::cout<<graph.get_freq(vs)<<" MMMMMMMMMM\n";
    CHECK(graph.get_freq(vs)==4);
    vs.clear();

    vs.push_back(2);
    //std::cout<<graph.get_freq(vs)<<" MMMMMMMMMM\n";
    CHECK(graph.get_freq(vs)== 4);
    vs.clear();

    vs.push_back(5);
    //std::cout<<graph.get_freq(vs)<<" MMMMMMMMMM\n";
    CHECK(graph.get_freq(vs)==6);
    vs.clear();

    vs.push_back(2);
    vs.push_back(3);
    //std::cout<<graph.get_freq(vs)<<" MMMMMMMMMM\n";
    CHECK(graph.get_freq(vs)==3);
    vs.clear();

    vs.push_back(5);vs.push_back(2);vs.push_back(3);vs.push_back(4);
    vs.push_back(6);
    CHECK(graph.get_freq(vs)==1);
    vs.clear();

    vector<uint32_t> suffix;
    vector<uint32_t> counts;
    vs.push_back(15);
    graph.get_suffix(vs, suffix, counts);
    CHECK(suffix.size()==2);
    CHECK(counts.size()==2);
    for (size_t i=0; i<suffix.size();++i)
    {
      if (suffix[i] == 13)
      {
        CHECK(counts[i]==2);
      }    
      else if(suffix[i]==17)
      {
        CHECK(counts[i]==3);
      }
      else
        CHECK(false);
    }
    vs.clear();

    vs.push_back(2);vs.push_back(3);
    graph.get_suffix(vs, suffix, counts);
    CHECK(suffix.size()==1);
    CHECK(counts.size()==1);
    for (size_t i=0; i<suffix.size();++i)
    {
      if (suffix[i]==4)
      {
        CHECK(counts[i]==2);
      }
      else
        CHECK(false);
    }
    vs.clear();
    
    vector<uint32_t> docids;
    graph.get_doc_list(suffix, docids);
    
  }

  system("rm -fr ./tt*");
  construct_trie("./tt", 100000);
  construct_trie("./tt1", 100000);  
  {
    Graph<> graph("./tt");
    Graph<> graph2("./tt1");

    struct timeval tvafter,tvpre;
    struct timezone tz;

    gettimeofday (&tvpre , &tz);
    graph.merge(graph2);
    gettimeofday (&tvafter , &tz);
    cout<<"\nMerge graph ("<<graph.doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";

    
    gettimeofday (&tvpre , &tz);
    graph.counter_merge(graph2);
    gettimeofday (&tvafter , &tz);
    cout<<"\nCounter-Merge graph ("<<graph.doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";

  }
  


}

int main()
{
  
  //   dyn_array_check<uint64_t>();
  //dyn_array_check<struct TEST_STRUCT>();

//    integer_hash_check<uint64_t>();
//    integer_hash_check<struct TEST_STRUCT>();

//    id_transfer_check();

  sorter_check();

  graph_merge_check();
  graph_check();
}

 
