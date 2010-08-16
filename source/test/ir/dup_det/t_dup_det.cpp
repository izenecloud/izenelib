
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
#include <ir/dup_det/integer_dyn_array.hpp>
#include <ir/dup_det/doc_fp_list.hpp>
#include <ir/dup_det/partial_fp_list.hpp>
#include <ir/dup_det/hash_table.hpp>
#include <ir/dup_det/hash_group.hpp>
#include <ir/dup_det/fp_hash_table.hpp>
#include <ir/dup_det/group_table.hpp>
#include <ir/dup_det/index_fp.hpp>
#include <string>
#include <time.h>
#include <math.h>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp> 
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp> 
#include <sys/time.h>
#include <fstream>
#include <iostream>
#include <vector>
#include<stdio.h>

#define CHECK(f)\
  {     \
    if (!(f)){ BOOST_CHECK(false); std::cout<<"ERROR: "<<__FILE__<<": "<<__LINE__<<": "<<__FUNCTION__<<endl;} \
  }
#define ERROR_COUNT {if(error_count>0)cout<<endl<<error_count<<" errors ware found!";else{cout<<"\nNo error detected!\n"}}

//USING_IZENE_LOG();

BOOST_AUTO_TEST_SUITE( t_duplication_detection_suite )

using namespace izenelib::ir;
using namespace izenelib::am;
using namespace std;
using namespace boost::unit_test;

//
bool match(const std::string& path, const char* prefix)
{
    uint32_t i = path.find_last_of('/');
    i = path.substr(i+1).find(prefix);
    if (i != (uint32_t)-1)
        return true;
    return false;
}

void remove(const char* prefix)
{
    boost::filesystem::path full_path(   "./"   ,boost::filesystem::native);
    if (boost::filesystem::exists(full_path))
         {
       boost::filesystem::directory_iterator item_begin(full_path);
       boost::filesystem::directory_iterator item_end;
       for ( ;item_begin   !=  item_end; item_begin ++ )
                 {
           if (match(item_begin ->path().native_file_string(), prefix))
        		boost::filesystem::remove(item_begin ->path().native_file_string());
           //cout  << item_begin ->path().native_file_string() << " \t[dir] " << endl;
                 }
        }
}

BOOST_AUTO_TEST_CASE(integer_dyn_array_check )
{
  const size_t SIZE=50000;
  typedef IntegerDynArray<uint64_t> Array;

  vector<uint64_t> v;
  for (size_t i=0; i<SIZE; i++)
    v.push_back(rand());

  Array ar(v);
  Array br = ar;

  BOOST_CHECK(ar == br);
  br[1000] = 222;
  v[1000] = 222;
  BOOST_CHECK(br == v);
  
  
  clock_t start, vt, at;
  vt = at = 0;
  
  for (size_t i=0; i<SIZE; i++)
  {
    uint64_t k = rand();

    start = clock();
    v.push_back(k);
    vt += clock()-start;

    start = clock();
    br.push_back(k);
    at += clock()-start;    
  }
  printf( "\n[push_back] vector: %f My Array: %f !\n", (double)(vt) / CLOCKS_PER_SEC, (double)(at) / CLOCKS_PER_SEC);

  br.compact();
  BOOST_CHECK(br == v);
  BOOST_CHECK(br.find(ar[100])==100);
  BOOST_CHECK(br.find(ar[1003])==1003);

  br = ar;

  
  br += v;
  
  Array cr(v);
  ar += cr;
  BOOST_CHECK(br == ar);
  

  //----------------------------------------------

  typedef IntegerDynArray<uint64_t, true> SortedArray;
  SortedArray sa;
  for (size_t i=0; i<SIZE/1000; i++)
  {
    sa.push_back(rand());
    //cout<<sa<<endl;
  }

  for (size_t i=0; i<sa.length()-1; i++)
    CHECK(sa[i]<=sa[i+1]);
  
  BOOST_CHECK(sa.find(sa[10])==10);
  BOOST_CHECK(sa.find(sa[5])==5);
  
}

BOOST_AUTO_TEST_CASE(partial_fp_list_check )
{
  const uint32_t FP_LENGTH= 12;
  #define THRESHOLD 2
  const size_t TYPES_NUM= 400;
  const size_t SIZE= 1000000;
  
  typedef PartialFpList<uint32_t, FP_LENGTH, 1> FpList;
  typedef FpList::FpVector FpVector;

  vector<FpVector>  v;
  v.reserve(TYPES_NUM);
  for (size_t i = 0; i<TYPES_NUM; i++)
  {
    FpVector vv;
    vv.reserve(FP_LENGTH);
    for (size_t j=0; j<FP_LENGTH; j++)
      vv.push_back(rand());
    
    v.push_back(vv);
  }

  cout<<"Data is ready!\n";
  clock_t start, finish;

  {
    FpList docs("./tt");
    docs.reset();
  
    start = clock();
    for (size_t i=0; i<SIZE/2; i++)
      docs.add_doc(i, v[i%TYPES_NUM]);

    cout<<"Flushing...\n";
    docs.flush();
    finish = clock();
    printf( "\nAdd docs (%d): %f \n",docs.doc_num(), (double)(finish-start) / CLOCKS_PER_SEC);
  }

  FpList docs("./tt");
  start = clock();
  
  for (uint8_t k=0; k<THRESHOLD+1; k++)
  {
    docs.ready_for_uniform_access(k);
    for (size_t i=0; i<docs.doc_num()-TYPES_NUM; i++)
      for (uint8_t j=0; j<docs.unit_len(); j++)
        if(!docs.get_fp(k, i, j)==docs.get_fp(k, i+TYPES_NUM, j))
          CHECK(false);
  }
  
  finish = clock();
  printf( "\nSimilar docs: %f \n", (double)(finish-start) / CLOCKS_PER_SEC);

  start = clock();
  for (size_t i=0; i<SIZE/2; i++)
    docs.add_doc(i, v[i%TYPES_NUM]);
  docs.flush();
  finish = clock();
  printf( "\nAdd docs (%d): %f \n",docs.doc_num(), (double)(finish-start) / CLOCKS_PER_SEC);

  
  start = clock();  
  for (uint8_t k=0; k<THRESHOLD+1; k++)
  {
    docs.ready_for_uniform_access(k);
    for (size_t i=0; i<docs.doc_num()-TYPES_NUM; i++)
      for (uint8_t j=0; j<docs.unit_len(); j++)
        if(!docs.get_fp(k, i, j)==docs.get_fp(k, i+TYPES_NUM, j))
          CHECK(false);
  }
  
  finish = clock();
  printf( "\nSimilar docs: %f \n", (double)(finish-start) / CLOCKS_PER_SEC);

  
}

void prime_gen(uint32_t s)
{
  for (uint32_t i = 3; i<s; ++i)
  {
    uint32_t k = (uint32_t)sqrt(i);
    if (k*k == i)
      continue;

    uint32_t j=3;
    for (; j<k; ++j)
      if (i%j==0)
        break;

    if (j == k)
    {
      //std::cout<<i<<std::endl;
      //i is prime number
    }
  }
}

// bool is_prime(uint32_t a, uint32_t b)
// {
  
//   uint32_t c=0;
  
//   while ((a%b)!=0)
//   {
//     c=a%b;
//     a=b;
//     b=c;
//   }
  
//   return b == 1;

// }

BOOST_AUTO_TEST_CASE(group_table_check )
{

  const uint32_t FP_LENGTH = 6;
  #define THRESHOLD 2
  const size_t TYPES_NUM= 11;
  
  typedef PartialFpList<> FpList;
  typedef FpList::FpVector FpVector;

  vector<FpVector>  v;
  v.reserve(TYPES_NUM);
  for (size_t i = 0; i<TYPES_NUM; i++)
  {
    FpVector vv;
    vv.reserve(FP_LENGTH);
    for (size_t j=0; j<FP_LENGTH; j++)
      vv.push_back(rand());
    
    v.push_back(vv);
  }

  boost::filesystem::remove_all("./tt.fp.0");
  boost::filesystem::remove_all("./tt.fp.1");
  boost::filesystem::remove_all("./tt.fp.2");
  boost::filesystem::remove_all("./tt.id");
  FpList fplist("./tt");
  for (size_t i = 0; i<TYPES_NUM; i++)
  {
    fplist.add_doc(i+TYPES_NUM, v[i]);
  }
  
  cout<<"Data is ready!\n";

  typedef izenelib::am::IntegerDynArray<uint32_t> Vector32;
  typedef izenelib::ir::GroupTable<> Group;
  boost::filesystem::remove_all("./group");

  {
    Group group("./group");
  
    group.add_doc(1, 3);
    group.add_doc(5, 1);
    group.add_doc(2, 4);
    group.add_doc(2, 6);
    group.add_doc(5, 7);
    group.add_doc(3, 9);
    group.add_doc(10, 8);
    group.add_doc(6, 8);

    BOOST_CHECK(group.same_group(3, 5));
    BOOST_CHECK(group.same_group(4, 6));
    BOOST_CHECK(!group.same_group(3, 6));

    group.flush();
  }

  Group group("./group");

  group.add_doc(3, 8);
  group.add_doc(2, 1);

  //cout<<group<<endl;

  BOOST_CHECK(group.exist(3));
  BOOST_CHECK(!group.exist(11));

  
  BOOST_CHECK(group.same_group(3, 5));
  BOOST_CHECK(group.same_group(1, 6));
  BOOST_CHECK(group.same_group(1, 10));
  BOOST_CHECK(!group.same_group(11, 6));

  group.set_docid(fplist);
  //cout<<group<<endl;
    
  BOOST_CHECK(group.same_group(3+TYPES_NUM, 5+TYPES_NUM));
  BOOST_CHECK(group.same_group(1+TYPES_NUM, 6+TYPES_NUM));
  BOOST_CHECK(group.same_group(1+TYPES_NUM, 10+TYPES_NUM));
  BOOST_CHECK(!group.same_group(11+TYPES_NUM, 6+TYPES_NUM));

  Vector32 r = group.find(11+TYPES_NUM);
  BOOST_CHECK(r.length()==0);
  r = group.find(1+TYPES_NUM);
  BOOST_CHECK(r.length()==10);
  //cout<<group<<endl;
  
  //getchar();
}

BOOST_AUTO_TEST_CASE(fp_hash_table_check )
{
  const size_t SIZE= 1000000;
  const uint32_t FP_LENGTH = 6;
  const size_t TYPES_NUM= 400000;
  
  typedef FpHashTable<> FpHT;
  typedef PartialFpList<> FpList;
  typedef FpList::FpVector FpVector;
  typedef izenelib::am::IntegerDynArray<uint8_t> Vector8;
  typedef izenelib::am::IntegerDynArray<uint32_t> Vector32;
  typedef izenelib::am::IntegerDynArray<uint64_t> Vector64;

  vector<FpVector>  v;
  v.reserve(TYPES_NUM);
  for (size_t i = 0; i<TYPES_NUM; i++)
  {
    FpVector vv;
    vv.reserve(FP_LENGTH);
    for (size_t j=0; j<FP_LENGTH; j++)
      vv.push_back(rand());
    
    v.push_back(vv);
  }
  cout<<"Data is ready!\n";

  
  boost::filesystem::remove_all ("./fp_hash.key");
  boost::filesystem::remove_all ("./fp_hash.fp");
  boost::filesystem::remove_all ("./fp_hash.has");
  FpHT fp("./fp_hash");
  struct timeval tvafter,tvpre;
  struct timezone tz;

  gettimeofday (&tvpre , &tz);
  fp.ready_for_insert();
  for (size_t i=0; i<SIZE; i++)
    fp.add_doc(i, v[i%TYPES_NUM]);

  fp.flush();
  gettimeofday (&tvafter , &tz);
  cout<<"\nAdd FP("<<fp.doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

  gettimeofday (&tvpre , &tz);
  fp.ready_for_find();
  //cout<<fp;
  for (size_t i=0; i<SIZE; i++)
  {
    const uint64_t* p = fp.find(i);
    // for (size_t j=0; j<FP_LENGTH; j++)
//       cout<<p[j]<<" ";
//     cout<<endl;
    
    if (p==NULL || !(v[i%TYPES_NUM] == p))
      CHECK(i!=i);
  }
  //cout<<fp;
  gettimeofday (&tvafter , &tz);
  cout<<"\nFind FP("<<fp.doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
  
}

BOOST_AUTO_TEST_CASE(hash_table_check )
{
  #define UNIT_LEN 2
  const size_t SIZE= 1000000;
  const size_t TYPES_NUM= 200000;
  
  typedef FpHashTable<> FpHT;
  typedef PartialFpList<> FpList;
  typedef FpList::FpVector FpVector;
  typedef izenelib::am::IntegerDynArray<uint8_t> Vector8;
  typedef izenelib::am::IntegerDynArray<uint32_t> Vector32;
  typedef izenelib::am::IntegerDynArray<uint64_t> Vector64;
  typedef HashTable<> HashT;

  vector<uint64_t>  v;
  v.reserve(TYPES_NUM);
  for (size_t i = 0; i<TYPES_NUM; i++)
    v.push_back(TYPES_NUM+i);
  
  cout<<"Data is ready!\n";

  struct timeval tvafter,tvpre;
  struct timezone tz;
  
  boost::filesystem::remove_all ("./fp_hash");
  {    
    HashT ht("./fp_hash");

    gettimeofday (&tvpre , &tz);
    for (size_t i=0; i<SIZE; i++)
      ht.push_back(v[i%TYPES_NUM]);

    ht.flush();
  }
  gettimeofday (&tvafter , &tz);
  cout<<"\nAdd doc: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

  
  gettimeofday (&tvpre , &tz);
  HashT ht("./fp_hash");
  //cout<<fp;
  for (size_t i=0; i<SIZE-TYPES_NUM; ++i)
  {
    const Vector32 p1 = ht[i];
    const Vector32 p2 = ht[i+TYPES_NUM];

    CHECK(p1 == p2);
    
    CHECK(p1.length() == SIZE/TYPES_NUM);
    
  }
  //cout<<fp;
  gettimeofday (&tvafter , &tz);
  cout<<"\nFind doc: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
  
}

BOOST_AUTO_TEST_CASE(overall_performace_check )
{
  cout<<"Checking performance ...\n";
  boost::filesystem::remove_all("./tt");
  boost::filesystem::remove_all("./fp");
  boost::filesystem::remove_all("./group");
  
  const uint32_t  FP_LENGTH = 12;
  const size_t SIZE= 20;//1000000;

  const size_t TYPES_NUM= 4;//40000;
  typedef izenelib::am::IntegerDynArray<uint64_t> Vector64;
  typedef izenelib::am::IntegerDynArray<uint32_t> Vector32;
  typedef izenelib::am::IntegerDynArray<uint8_t> Vector8;

  vector<Vector32>  v;
  v.reserve(TYPES_NUM);
  for (size_t i = 0; i<TYPES_NUM; i++)
  {
    Vector8 v8;
    v8.reserve(48);
    for (size_t j=0; j<48; j++)
      v8.push_back(rand());

    Vector32 v32((const char*)v8.data(),v8.size());
    v.push_back(v32);
  }

  cout<<"Data is ready!\n";
  boost::filesystem::remove_all("./tt");

  struct timeval tvafter,tvpre;
  struct timezone tz;

//   gettimeofday (&tvpre , &tz);
//   prime_gen(5000000);
//   gettimeofday (&tvafter , &tz);
//   cout<<"Prime number generation: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" mins\n";

  
  FpIndex<1, uint32_t, FP_LENGTH> fp_index("./tt", 1);
  gettimeofday (&tvpre , &tz);
  
  fp_index.ready_for_insert();
  
  for (size_t i=0; i<SIZE; i++)
    fp_index.add_doc(i+SIZE, v[i%TYPES_NUM]);
  gettimeofday (&tvafter , &tz);
  cout<<"Adding docs is over! "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
  //  getchar();
  
  gettimeofday (&tvpre , &tz);
  fp_index.indexing();
  fp_index.flush();
  gettimeofday (&tvafter , &tz);
  cout<<"Indexing docs is over! "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
  //getchar();

  //test for incremental
  {
    fp_index.ready_for_insert();
    
    for (size_t i=SIZE; i<SIZE+2*TYPES_NUM; i++)
      fp_index.add_doc(i+SIZE, v[i%TYPES_NUM]);

    fp_index.indexing(SIZE);
    fp_index.flush();
  }

  //test for updating
  {
    vector<uint32_t> docids;
    docids.push_back(SIZE+2);
    docids.push_back(SIZE+3);
    docids.push_back(SIZE+4);
    vector<Vector32> fps;
    fps.push_back(v[0]);
    fps.push_back(v[0]);
    fps.push_back(v[0]);
    
    fp_index.update_docs(docids, fps);
    fps.clear();
    
    fps.push_back(v[2%TYPES_NUM]);
    fps.push_back(v[3%TYPES_NUM]);
    fps.push_back(v[4%TYPES_NUM]);

    fp_index.update_docs(docids, fps);

    docids.clear();
    docids.push_back(SIZE+5);
    fp_index.del_docs(docids);

    fp_index.ready_for_insert();
    fp_index.add_doc(5+SIZE, v[5%TYPES_NUM]);
    fp_index.indexing(fp_index.doc_num()-1);
    fp_index.flush();
  }
  
  
  for (size_t i=0; i<SIZE+TYPES_NUM; ++i)
  {
    const Vector32 p1 = fp_index.find(i+SIZE);
    const Vector32 p2 = fp_index.find(i+SIZE+TYPES_NUM);

    CHECK(p1 == p2);
    
    CHECK(p1.length() == SIZE/TYPES_NUM+2);

//     if (!(p1 == p2))
//     {
//       cout<<i<<"  -------------\n";
//       cout<<p1<<endl;
//       cout<<p2<<endl;
//       cout<<"=============\n";
//       break;
//     }
    
  }
    remove("tt");
    remove("fp_");

}


// BOOST_AUTO_TEST_CASE(overall_performace_check )
// {
// #define FP_LENGTH 6
// const int TYPES_NUM= 5000000
//   typedef izenelib::am::IntegerDynArray<uint64_t> Vector64;

//   vector<Vector64>  v;
//   v.reserve(TYPES_NUM);
//   for (size_t i = 0; i<TYPES_NUM; i++)
//   {
//     Vector64 vv;
//     vv.reserve(FP_LENGTH);
//     for (size_t j=0; j<FP_LENGTH; j++)
//       vv.push_back(rand());
    
//     v.push_back(vv);
//   }

//   cout<<"Data is ready!\n";
//   boost::filesystem::remove_all("./tt*");

//   struct timeval tvafter,tvpre;
//   struct timezone tz;

  
//   FpIndex<> fp_index("./tt");
//   gettimeofday (&tvpre , &tz);
  
//   fp_index.ready_for_insert();
  
//   for (size_t i=0; i<SIZE; i++)
//     fp_index.add_doc(i, v[i%TYPES_NUM]);
//   cout<<"Flushing...\n";
//   fp_index.flush();
//   gettimeofday (&tvafter , &tz);
//   cout<<"Adding docs is over! "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
//   //  getchar();

//   gettimeofday (&tvpre , &tz);
//   fp_index.indexing2();
//   fp_index.flush();
//   gettimeofday (&tvafter , &tz);
//   cout<<"Indexing docs is over! "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
//   getchar();

// }

BOOST_AUTO_TEST_SUITE_END()
