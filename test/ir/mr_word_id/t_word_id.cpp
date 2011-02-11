
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
#include <ir/mr_word_id/term_freq.hpp>
#include <ir/mr_word_id/id_str_table.hpp>
#include <ir/mr_word_id/mister_wordID.hpp>
#include <ir/mr_word_id/bigram_freq.hpp>
#include <ir/mr_word_id/id_table.hpp>
#include <ir/mr_word_id/str_str_table.hpp>

#include <string>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <fstream>
#include <iostream>
#include <vector>
#include<stdio.h>
#include <boost/bind.hpp>


uint32_t error_count = 0;

#define CHECK(f)\
  {                                                \
    if (!(f)){ ++error_count; std::cout<<"ERROR: "<<__FILE__<<": "<<__LINE__<<": "<<__FUNCTION__<<endl;} \
  }
#define ERROR_COUNT {if(error_count>0)cout<<endl<<error_count<<" errors ware found!";else{cout<<"\nNo error detected!\n"}}

using namespace izenelib::ir;

using namespace std;

string rand_str()
{
  string r;
  uint32_t len = rand()%20;
  while (len == 0)
    len = rand()%20;

  for (uint32_t i=0; i<len; ++i)
    r += 'a' + rand()%26;

  return r;
}

string rand_str1()
{
  string r;
  uint32_t len = rand()%80;

  for (uint32_t i=0; i<len; ++i)
    r += rand()%128;

  return r;
}

//
void check_term_freq_correctness()
{
  std::system("rm -f ./tt*");
  
  vector<string> dic;
  dic.push_back("hello");
  dic.push_back("Kevin");
  dic.push_back("kevin");
  dic.push_back("大方");
  dic.push_back("Kevin");
  dic.push_back("izene");
  dic.push_back("Izene");
  dic.push_back("software");
  dic.push_back("可爱");
  dic.push_back("大方");
  dic.push_back("hello");
  dic.push_back("hello");
  dic.push_back("可爱");

  typedef TermFrequency<uint8_t, 1> table_t;

  {
    table_t t;
    for (uint32_t i=0; i<dic.size(); ++i)
      t.increase(dic[i]);

    //t.display(cout);
    
    CHECK(t.find("hello") == 3);
    CHECK(t.find("Kevin") == 2);
    CHECK(t.find("kevin") == 1);
    CHECK(t.find("大方") == 2);
    CHECK(t.find("可爱") == 2);
    CHECK(t.find("gggg") == 0);

    FILE* f = fopen("./tt", "w+");
    t.save(f);
    fclose(f);
  }
  
  {
    table_t t;
    FILE* f = fopen("./tt", "r+");
    t.load(f);
    fclose(f);

    //t.display(cout);
    CHECK(t.find("hello") == 3);
    CHECK(t.find("Kevin") == 2);
    CHECK(t.find("kevin") == 1);
    CHECK(t.find("大方") == 2);
    CHECK(t.find("可爱") == 2);
    CHECK(t.find("gggg") == 0);
    CHECK(t.num_items() == 8);

    CHECK(t.begin());
    uint32_t freq = 0;
    string str;
    while(t.next(str, freq))
      cout<<str<<":"<<freq<<endl;
  }
}

void term_freq_performance_check()
{
  const uint32_t SIZE= 1000000;

  typedef TermFrequency<> table_t;

  clock_t start, finish;

  {
    table_t t;

    start = clock();
    for (uint32_t i=0; i<SIZE; ++i)
      t.increase(rand_str());
    finish = clock();
    printf( "\nIncrease random strings (%d): %f \n",SIZE, (double)(finish-start) / CLOCKS_PER_SEC);
  }

  vector<string> dics;
  for (uint32_t i = 0; i<SIZE; ++i)
    dics.push_back(rand_str());
  
  {
    table_t t;
    start = clock();
    for (uint32_t i=0; i<SIZE; ++i)
      t.insert(dics[i], (uint32_t)(dics[i][0]+1));
    finish = clock();
    printf( "\nInsert random strings (%d): %f \n",SIZE, (double)(finish-start) / CLOCKS_PER_SEC);

    FILE* f = fopen("./tt", "w+");
    t.save(f);
    fclose(f);
  }

  {
    table_t t;
    FILE* f = fopen("./tt", "r+");
    t.load(f);
    fclose(f);

    start = clock();
    for (uint32_t i=0; i<SIZE; ++i)
      CHECK(t.find(dics[i])==(uint32_t)(dics[i][0]+1));
    finish = clock();
    printf( "\nFind random strings (%d): %f \n",SIZE, (double)(finish-start) / CLOCKS_PER_SEC);
  }
  
}

void id_string_table_check()
{
  std::system("rm -f ./tt*");

  vector<string> dic;
  const uint32_t SIZE= 20;
  for (uint32_t i=0; i<SIZE; ++i)
    dic.push_back(rand_str1());
  
  {
    IdStringTable<uint32_t, uint16_t, 1> t;

    std::string str;
    uint32_t len = 0;

    CHECK(!t.update(94, len, str.c_str()));
    CHECK(t.find(94, len)==NULL);
    CHECK(!t.remove(94));

    for (uint32_t i=0; i<SIZE; ++i)
    {
      //cout<<dic[i].length()<<",";
      t.insert(i, dic[i].length(), dic[i].c_str());
    }
    //cout<<endl;

    //t.display();
    for (uint32_t i=0; i<SIZE; ++i)
    {
      const char*  b = t.find(i, len);
      CHECK(string(b, len) == dic[i]);
    }

    t.remove(0);
    t.remove(5);
    t.remove(8);
    t.insert(0, dic[0].length(), dic[0].c_str());
    t.insert(5, dic[5].length(), dic[5].c_str());
    t.insert(8, dic[8].length(), dic[8].c_str());

    t.update(0, dic[1].length(), dic[1].c_str());
    t.update(5, dic[6].length(), dic[6].c_str());
    t.update(8, dic[9].length(), dic[9].c_str());

    t.update(0, dic[0].length(), dic[0].c_str());
    t.update(5, dic[5].length(), dic[5].c_str());
    t.update(8, dic[8].length(), dic[8].c_str());

    FILE* f = fopen("./tt", "w+");
    t.save(f);
    fclose(f);
  }
  
  {
    IdStringTable<uint32_t, uint16_t, 1> t;
    FILE* f = fopen("./tt", "r+");
    t.load(f);
    fclose(f);

    //t.display();
    
    uint32_t len = 0;
    for (uint32_t i=0; i<SIZE; ++i)
    {
      const char*  b = t.find(i, len);
      if (b == NULL)
      {
        CHECK(false);
        break;
      }
      CHECK(string(b, len) == dic[i]);
    }
  }
}

void id_string_table_performance_check()
{
  std::system("rm -f ./tt*");

  vector<string> dic;
  const uint32_t SIZE= 1000000;
  for (uint32_t i=0; i<SIZE; ++i)
    dic.push_back(rand_str1());

  std::cout<<"Data is ready!\n";
  clock_t start, finish;
  {
    IdStringTable<> t;

    std::string str;
    uint32_t len = 0;

    CHECK(!t.update(94, len, str.c_str()));
    CHECK(t.find(94, len)==NULL);
    CHECK(!t.remove(94));

    start = clock();
    for (uint32_t i=0; i<SIZE; ++i)
    {
      //cout<<dic[i].length()<<",";
      t.insert(i, dic[i].length(), dic[i].c_str());
    }
    finish = clock();
    printf( "\nIdStringTable strings insertion(%d): %f \n",SIZE, (double)(finish-start) / CLOCKS_PER_SEC);
    //cout<<endl;

    //t.display();
    for (uint32_t i=0; i<SIZE; ++i)
    {
      const char*  b = t.find(i, len);
      CHECK(string(b, len) == dic[i]);
    }

    FILE* f = fopen("./tt", "w+");
    t.save(f);
    fclose(f);
  }
  
  {
    IdStringTable<> t;
    FILE* f = fopen("./tt", "r+");
    t.load(f);
    fclose(f);

    //t.display();
    
    uint32_t len = 0;
    start = clock();
    for (uint32_t i=0; i<SIZE; ++i)
    {
      const char*  b = t.find(i, len);
      if (b == NULL)
      {
        CHECK(false);
        break;
      }
      CHECK(string(b, len) == dic[i]);
    }
    finish = clock();
    printf( "\nIdStringTable strings search(%d): %f \n",SIZE, (double)(finish-start) / CLOCKS_PER_SEC);
  }
}

vector<string> eng_participle(const string& line)
{
  vector<string> r;
  uint32_t s = 0;
  uint32_t e = 0;

  for (uint32_t i=0; i<line.length(); ++i)
  {
    if (line[i]>='a' && line[i]<='z'
        ||line[i]>='A' && line[i]<='Z')
    {
      ++e;
      continue;
    }

    if (e-s <=1)
    {
      ++e;
      s= e;
      continue;
    }

    string str = line.substr(s, e-s);
    for (uint32_t i=0; i<str.length(); ++i)
      if(str[i]>='A' && str[i]<='Z')
        str[i] += 'a'-'A';
    
    r.push_back(str);
    //std::cout<<line.substr(s, e-s)<<std::endl;
    ++e;
    s = e;
  }

  return r;
}

void mister_wordID_check()
{
  std::system("rm -f ./tt*");
  {
    MisterWordID<> wordid("./tt");
    if (!wordid.is_ready())
      wordid.prepareID("dictionary.txt",
                       "/home/Kevin/sf1-revolution/bin/collection/english-wiki/scd/index/"
                       ,eng_participle);

    for (uint32_t i=0; i<50; ++i)
      std::cout<<wordid.get_word(i)<<std::endl;
  }
  
}

void bigram_freq_check()
{
  std::system("rm -f ./tt*");
  {
    BigramFrequency<> table("./tt");
  }

  const uint32_t SIZE = 1000000                                                      ;
  vector<uint32_t> id1s;
  vector<uint32_t> id2s;
  vector<uint32_t> fre;

  for (uint32_t i=0; i<SIZE; i++)
  {
    id1s.push_back(i++);
    id2s.push_back(i);
    fre.push_back(0);
  }

  //std::cout<<id1s.size()<<std::endl;

  clock_t start, finish;
  {
    BigramFrequency<> table("./tt");
    start = clock();
    for (uint32_t i=0; i<SIZE;)
    {
      table.set_start(i);
      table.set_end(i+SIZE/3);
      i+=SIZE/3+1;

      for (uint32_t j=0; j<SIZE; ++j)
      {
        uint32_t idx = rand()%id1s.size();
        if(table.update(id1s[idx], id2s[idx]))
          ++fre[idx];
      }

      //table.display();
      table.flush();
    }
    finish = clock();
    printf( "\nBigramFrequency updating(%d): %f \n",SIZE, (double)(finish-start) / CLOCKS_PER_SEC);
  }
  
  {
    BigramFrequency<> table("./tt");

    table.load();
    //table.display();

    start = clock();
    for (uint32_t i=0; i<id1s.size(); ++i)
    {
      //std::cout<<id1s[i]<<" "<<fre[i]<<" "<<table.find(id1s[i],id2s[i])<<std::endl;
      CHECK(table.find(id1s[i],id2s[i]) == fre[i]);
    }
    finish = clock();
    printf( "\nBigramFrequency find(%d): %f \n",SIZE, (double)(finish-start) / CLOCKS_PER_SEC);
  }
    
  {
    BigramFrequency<> table("./tt");

    start = clock();
    double thr = table.optimize();
    finish = clock();
    printf( "\nBigramFrequency optimizing(%d): %f \n",SIZE, (double)(finish-start) / CLOCKS_PER_SEC);

    table.load();
    //table.display();
    
    for (uint32_t i=0; i<id1s.size(); ++i)
    {
      if (fre[i]<thr)
        continue;
      //std::cout<<id1s[i]<<" "<<fre[i]<<" "<<table.find(id1s[i],id2s[i])<<std::endl;
      CHECK(table.find(id1s[i],id2s[i]) == fre[i]);
    }
  }  
}

void id_table_check()
{
  std::system("rm -f ./tt*");

  const uint32_t SIZE = 10000000                                                      ;
  vector<uint32_t> id1s;
  vector<uint32_t> id2s;

  for (uint32_t i=0; i<SIZE; i++)
  {
    id1s.push_back(rand());
    id2s.push_back(id1s[id1s.size()-1]+1);
  }

  //std::cout<<id1s.size()<<std::endl;

  clock_t start, finish;
  {
    IdTable<> table;
    start = clock();
    for (uint32_t i=0; i<SIZE; ++i)
      table.insert(id1s[i], id2s[i]);
    finish = clock();
    printf( "\nIdTable insertion(%d): %f \n",SIZE, (double)(finish-start) / CLOCKS_PER_SEC);

    FILE* f = fopen("./tt", "w+");
    table.save(f);
    fclose(f);
  }
 
  {
    IdTable<> table;
    FILE* f = fopen("./tt", "r+");
    table.load(f);
    fclose(f);

    start = clock();
    for (uint32_t i=0; i<SIZE; i+=2)
      table.remove(id1s[i]);
    finish = clock();
    printf( "\nIdTable remove(%d): %f \n",SIZE/2, (double)(finish-start) / CLOCKS_PER_SEC);

    f = fopen("./tt", "w+");
    table.save(f);
    fclose(f);
  }
 
  {
    IdTable<> table;
    FILE* f = fopen("./tt", "r+");
    table.load(f);
    fclose(f);

    start = clock();
    for (uint32_t i=1; i<SIZE; i+=2)
      table.update(id1s[i], rand());
    finish = clock();
    printf( "\nIdTable update(%d): %f \n",SIZE/2, (double)(finish-start) / CLOCKS_PER_SEC);

    for (uint32_t i=1; i<SIZE; i+=2)
      table.update(id1s[i], id2s[i]);

    for (uint32_t i=0; i<SIZE; ++i)
      table.insert(id1s[i], id2s[i]);
    f = fopen("./tt", "w+");
    table.save(f);
    fclose(f);
  }
  
  {
    IdTable<> table;
    FILE* f = fopen("./tt", "r+");
    table.load(f);
    fclose(f);

    start = clock();
    for (uint32_t i=0; i<SIZE; ++i)
      CHECK(table.find(id1s[i])==id2s[i]);
    finish = clock();
    printf( "\nIdTable find(%d): %f \n",SIZE, (double)(finish-start) / CLOCKS_PER_SEC);
  }
}

void str_str_table_check()
{
  std::system("rm -f ./tt*");

  const uint32_t SIZE = 1000000                                                      ;
  vector<string> str1s;
  vector<string> str2s;

  for (uint32_t i=0; i<SIZE; i++)
  {
    str1s.push_back(rand_str());
    str2s.push_back(rand_str());
  }

  //std::cout<<str1s.size()<<std::endl;

  clock_t start, finish;
  {
    StrStrTable<> table("./tt");
    start = clock();
    for (uint32_t i=0; i<SIZE; ++i)
    {
      //std::cout <<table.find(str1s[i])<<std::endl;
      table.insert(str1s[i], str2s[i]);
    }
    
    finish = clock();
    printf( "\nStrStrTable insertion(%d): %f \n",SIZE, (double)(finish-start) / CLOCKS_PER_SEC);
    
    table.flush();
  }
 
  {
    StrStrTable<> table("./tt");
    table.load();

    start = clock();
    for (uint32_t i=0; i<SIZE; i+=2)
      table.remove(str1s[i]);
    finish = clock();
    printf( "\nStrStrTable remove(%d): %f \n",SIZE/2, (double)(finish-start) / CLOCKS_PER_SEC);
  }
 
  {
    StrStrTable<> table("./tt");
    table.load();

    start = clock();
    for (uint32_t i=1; i<SIZE; i+=2)
      table.update(str1s[i], rand_str());
    finish = clock();
    printf( "\nStrStrTable update(%d): %f \n",SIZE/2, (double)(finish-start) / CLOCKS_PER_SEC);

    for (uint32_t i=1; i<SIZE; i+=2)
      table.update(str1s[i], str2s[i]);

    for (uint32_t i=0; i<SIZE; ++i)
      table.insert(str1s[i], str2s[i]);
  }
  
  {
    StrStrTable<> table("./tt");
    table.load();

    start = clock();
    for (uint32_t i=0; i<SIZE; ++i)
    {
      //std::cout <<table.find(str1s[i])<<"-"<<str2s[i]<<std::endl;
      CHECK(table.find(str1s[i]).find(str2s[i])!=(std::size_t)(-1));
    }
    
    finish = clock();
    printf( "\nStrStrTable find(%d): %f \n",SIZE, (double)(finish-start) / CLOCKS_PER_SEC);
  }
}


void bigram_freqency()
{
  struct timeval tvafter,tvpre;
  struct timezone tz;
  {
    MisterWordID<> wordid("./dic");
    if (!wordid.is_ready())
      wordid.prepareID("dictionary.txt",
                       "/home/Kevin/sf1-revolution/bin/collection/english-wiki/scd/index/",
                       eng_participle, "stop_words.txt");

    std::cout<<"\nDictionary is ready!\n";
    std::cout<<"Word count: "<<wordid.num_items()<<std::endl;

    namespace fs = boost::filesystem;
    const static uint32_t bs = 1000000;
    char* buff = new char[bs];
    
    fs::path full_path("/home/Kevin/sf1-revolution/bin/collection/english-wiki/scd/index/"
                       , fs::native);

    BigramFrequency<> bigr_fre("./bigram_freq");

    gettimeofday (&tvpre , &tz);
    if(!bigr_fre.load())
    {      
      uint32_t start = 0;
      uint32_t interval = 100000;
      while(start < wordid.num_items())
      {
        bigr_fre.set_start(start);
        bigr_fre.set_end(start+interval);
        start+= interval+1;
        
        fs::directory_iterator item_b(full_path);
        fs::directory_iterator item_e;
        char* buff = new char[bs];
        for (; item_b!=item_e; item_b++)
        {
          if (fs::is_directory(*item_b))
            continue;

          std::string nm = item_b->path().file_string();
          FILE* f = fopen(nm.c_str(), "r");
      
          fseek(f, 0, SEEK_END);
          uint64_t fs = ftell(f);
          fseek(f, 0, SEEK_SET);
          const uint64_t FS = fs;

          std::cout<<std::endl;
          while (fs > 0)
          {
            std::cout<<"\rScanning ... "<<(1.-(fs*1./FS))*100.0<<"%"<<std::flush;
            uint32_t rs = bs > fs? fs: bs;
            IASSERT(fread(buff, rs, 1, f)==1);
            fs -= rs;

            //std::cout<<fs<<" "<<rs<<std::endl;
            uint32_t pos_s = 0;
            uint32_t pos_e = 0;
            std::vector<std::string> terms;
            while (pos_s < rs && pos_e<=rs)
            {
              if (buff[pos_e]!='\n' && !(fs ==0 && pos_e==rs))
              {
                ++pos_e;
                continue;
              }

              if (pos_e - pos_s <=1)
              {
                ++pos_e;
                pos_s = pos_e;
                continue;
              }
      
              std::string line(&buff[pos_s], pos_e - pos_s);
              terms = eng_participle(line);
              
              for (uint32_t i=0, j=1; j<terms.size(); ++j, ++i)
              {
                //std::cout<<terms[i]<<"-"<<terms[j]<<std::endl;
                
                uint32_t id1 = wordid.get_id(terms[i]);
                if (id1 == 0 )
                  continue;

                uint32_t id2 = wordid.get_id(terms[j]);
                if (id2 == 0 )
                  continue;
                
                bigr_fre.update(id1, id2);
              }

              ++pos_e;
              pos_s = pos_e;
            }
        
            fseek(f, -1*(int)(pos_e-pos_s), SEEK_CUR);
            fs += pos_e - pos_s;
          }

          fclose(f);
        }
        bigr_fre.flush();
      }
    
      delete buff;
    }
    gettimeofday (&tvafter , &tz);
    cout<<"\nFrequency calculation is over! "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" mins\n";

    gettimeofday (&tvpre , &tz);
    bigr_fre.optimize();
    gettimeofday (&tvafter , &tz);
    cout<<"\nOptimize is over! "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" mins\n";
  }

  clock_t start, finish;
  {

    MisterWordID<> wordid("./dic");
    if (!wordid.is_ready())
      std::cout<<"\nDictionary is loaded!\n";
    
    BigramFrequency<> bigr_fre("./bigram_freq");
    
    if(!bigr_fre.load())
      std::cout<<"Bigram frequency is loaded!\n";

    char buf[256];
    while(1)
    {
      cout<<"Your first words: ";
      cin>>buf;

      start = clock();
      uint32_t id1 = wordid.get_id(buf);

      cout<<"\nCandidates: ";
      vector<uint32_t> id2s = bigr_fre.find(id1);
      for (uint32_t i = 0; i<id2s.size(); ++i)
        cout<<wordid.get_word(id2s[i])<<" ";
      cout<<endl;

      finish = clock();
      printf( "\nQuery time: %f sec\n", (double)(finish-start) / CLOCKS_PER_SEC);
    }
    
  }
}

int main()
{
  // check_term_freq_correctness();
//   term_freq_performance_check();
  
//   id_string_table_check();
//   id_string_table_performance_check();

//   eng_participle("In 1951, David A. Huffman and     his A prefix-free binary code (a set of codewords) Alphabet A = -left-{a_{1},a_{2},-cdots,a_{n}+right-}, which   ");

//   mister_wordID_check();

   bigram_freq_check();

//   id_table_check();

//   str_str_table_check();

//  bigram_freqency();
  
  //std::system("rm -f ./tt*");
}

