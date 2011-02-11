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
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <iostream>

#include <am/external_sort/sort_runner.hpp>
#include <am/external_sort/sort_merger.hpp>
#include <am/external_sort/izene_sort.hpp>
#include <sys/time.h>
#include <signal.h>

//USING_IZENE_LOG();

using namespace izenelib::am;
using namespace std;

BOOST_AUTO_TEST_SUITE(izene_sort_test)

uint32_t error_count = 0;
#define CHECK(f)\
  {     \
    if (!(f)){ BOOST_CHECK(false);++error_count; std::cout<<"ERROR: "<<__FILE__<<": "<<__LINE__<<": "<<__FUNCTION__<<endl;} \
  }
#define ERROR_COUNT {if(error_count>0)cout<<endl<<error_count<<" errors ware found!";else{cout<<"\nNo error detected!\n"}}

template<class T>
std::vector<char> randstr(T key)
{
  uint32_t len = rand()%100+sizeof(T);
  while(len == sizeof(T))
    len = rand()%100+sizeof(T);

  std::vector<char> str;
  str.reserve(len);
  for (uint32_t i=0; i<len; ++i)
    str.push_back('a'+rand()%26);
  
  *(T*)str.data() = key;
  return str;
}

template<class KEY_TYPE, class LEN_TYPE>
void check_runner(const uint64_t SIZE, uint32_t bs=100000000)
{
  //system("rm -f ./tt*");

  std::vector<char> str;
  FILE* f = fopen("./tt", "w+");
  fwrite(&SIZE, sizeof(uint64_t), 1, f);
  for (uint32_t i=0; i<SIZE; ++i)
  {
    str = randstr<KEY_TYPE>(SIZE-i);
    LEN_TYPE len = str.size();
    fwrite(&len, sizeof(LEN_TYPE), 1, f);
    fwrite(str.data(), len, 1, f);
    //cout<<"\rAdd data: "<<(double)i/SIZE*100.<<"%"<<std::flush;
  }
  fclose(f);  
  
  
  cout<<"\n-----------------------------\n";
  SortRunner<KEY_TYPE> runner("./tt", bs);
  runner.run();
  cout<<"\nRun number: "<<runner.run_num()<<endl;

  f = fopen("./tt", "r");
  cout<<"\n-----------------------------\n";
  uint64_t count = 0;
  fread(&count, sizeof(uint64_t), 1, f);
  CHECK(count == SIZE);
  fseek(f, 0, SEEK_END);
  uint64_t file_len = ftell(f);
  fseek(f, sizeof(uint64_t), SEEK_SET);

  uint64_t pos = sizeof(uint64_t);
  while(pos < file_len)
  {
    uint32_t run_num = 0;
    uint32_t run_size = 0;
    uint64_t next_run_pos = 0;
    fread(&run_size, sizeof(uint32_t), 1, f);
    fread(&run_num, sizeof(uint32_t), 1, f);
    fread(&next_run_pos, sizeof(uint64_t), 1, f);
    pos += sizeof(uint32_t)*2+sizeof(uint64_t);
    KEY_TYPE last = 0;
    for (uint32_t i=0; i<run_num; ++i)
    {
      LEN_TYPE len = 0;
      fread(&len, sizeof(len), 1, f);
      char* buf = new char[len];
      fread(buf, len, 1, f);
      if (i==0)
      {
        last = *(KEY_TYPE*)buf;
        pos += len + sizeof(len);
        run_size -= (len + sizeof(len));
        delete buf;
        continue;
      }
      CHECK(last <= *(KEY_TYPE*)buf);
      CHECK(1 == *(KEY_TYPE*)buf - last);
      if (last > *(KEY_TYPE*)buf || 1 != *(KEY_TYPE*)buf - last)
      {
        cout<<"ERROR: "<<*(KEY_TYPE*)buf<<"-"<<last<<endl;
        return;
      }

      last = *(KEY_TYPE*)buf;
      pos += len + sizeof(len);
      run_size -= (len + sizeof(len));
      delete buf;
    }
    CHECK(run_size == 0);
  }
}

template<class KEY_TYPE, class LEN_TYPE>
void check_merger(const uint64_t SIZE, uint32_t bs=100000000)
{
  //system("rm -f ./tt*");

  std::vector<char> str;
  FILE* f = fopen("./tt", "w+");
  fwrite(&SIZE, sizeof(uint64_t), 1, f);

  uint32_t run_num = rand()%300;
  while (run_num<100 || SIZE%run_num!=0)
    run_num = rand()%300;

  //run_num = 800;
  for (uint32_t i=0; i<run_num; ++i)
  {
    uint64_t pos = ftell(f);
    fseek(f, 2*sizeof(uint32_t)+sizeof(uint64_t), SEEK_CUR);
    uint32_t s = 0;
    for (uint32_t j=0; j<SIZE/run_num; ++j)
    {
      str = randstr<KEY_TYPE>(i*SIZE/run_num+j);
      LEN_TYPE len = str.size();
      fwrite(&len, sizeof(LEN_TYPE), 1, f);
      fwrite(str.data(), len, 1, f);
      s += len+sizeof(LEN_TYPE);

      //cout<<"\rAdd data: "<<(double)(i*SIZE/run_num+j)/SIZE*100.<<"%"<<std::flush;
    }
    uint64_t nextRunPos = ftell(f);
    fseek(f, pos, SEEK_SET);
    fwrite(&s, sizeof(uint32_t), 1, f);
    s = SIZE/run_num;
    fwrite(&s, sizeof(uint32_t), 1, f);
    fwrite(&nextRunPos, sizeof(uint64_t), 1, f);
    fseek(f, 0, SEEK_END);
  }
  fclose(f);  
  
  
  cout<<"\n-----------------------------\n";
  cout<<"Run number: "<<run_num<<endl;
  SortMerger<KEY_TYPE, LEN_TYPE> merger("./tt", run_num, bs, 2);
  merger.run();

  f = fopen("./tt", "r");
  uint64_t count = 0;
  fread(&count, sizeof(uint64_t), 1, f);
  CHECK(count == SIZE);
  for (uint32_t i=0; i<count; ++i)
  {
    LEN_TYPE len = 0;
    fread(&len, sizeof(LEN_TYPE), 1, f);
    char* buf = new char[len];
    fread(buf, len, 1, f);
    CHECK(*(KEY_TYPE*)buf == i);
    if (*(KEY_TYPE*)buf != i)
    {
      std::cout<<"\nERROR: "<<*(KEY_TYPE*)buf<<":"<<i<<std::endl;
      break;
    }
    delete buf;
  }
}


template<class KEY_TYPE, class LEN_TYPE>
void check_izene_sort(uint32_t SIZE = 200000, uint32_t bs =1000000)
{
  boost::filesystem::remove("./tt");
  boost::filesystem::remove("./tt.out");
  
  struct timeval tvafter, tvpre;
  struct timezone tz;
  
  IzeneSort<KEY_TYPE, LEN_TYPE, true> sorter("./tt", bs);

  cout<<"\n-----------------------------\n";
  std::vector<char> str;
  gettimeofday (&tvpre , &tz);
  for (uint32_t i=0; i<SIZE; ++i)
  {
    str = randstr<KEY_TYPE>(SIZE-i);
    sorter.add_data(str.size(), str.data());
    //cout<<"\rAdd data: "<<(double)i/SIZE*100.<<"%"<<std::flush;
  }
  gettimeofday (&tvafter , &tz);
  printf( "\nIt takes %f minutes to add %d random data!\n",
          ((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000.)/60000,
          SIZE);
  
  sorter.sort();

  char* data;
  LEN_TYPE len;
  KEY_TYPE i = 1;

  CHECK(sorter.begin());
  while (sorter.next_data(len, &data))
  {
    CHECK(i==*(KEY_TYPE*)data);
    if (i!=*(KEY_TYPE*)data)
    {
      //std::cout<<i<<"-"<<*(KEY_TYPE*)data<<endl;
      break;
    }
    free(data);
    ++i;
  }
}

BOOST_AUTO_TEST_CASE(izenesort_runner_test)
{
    check_runner<uint32_t, uint8_t>(1000, 1000000);
    check_runner<uint32_t, uint8_t>(10000, 1000000);
}

BOOST_AUTO_TEST_CASE(izenesort_merger_test)
{
    check_merger<uint32_t, uint8_t>(1000, 100000);
    check_merger<uint32_t, uint8_t>(10000, 1000000);
}

BOOST_AUTO_TEST_CASE(izenesort_test)
{
  check_izene_sort<uint64_t, uint16_t>(1000, 100000);
  check_izene_sort<uint32_t, uint8_t>(10000, 1000000);
  check_izene_sort<uint32_t, uint8_t>(2, 1000);
}

BOOST_AUTO_TEST_SUITE_END()



