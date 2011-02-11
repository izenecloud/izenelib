
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
#include <am/trie_index/term_hash_table1.hpp>
#include <am/trie_index/hash_trie1.hpp>
#include <util/hashFunction.h>
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

BOOST_AUTO_TEST_SUITE( t_term_hash_table_suite )

using namespace izenelib::am;
using namespace izenelib::util;
using namespace std;
using namespace boost::unit_test;

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

void writeDict(const char* input, vector<string>& vs, uint32_t s = 1000000)
{
  vector<string> v;
  readDict(input, v);
  vs.clear();
  
  for (uint32_t i=0; i<s; ++i)
  {
    vs.push_back(v[rand()%v.size()]);
  }
}

// BOOST_AUTO_TEST_CASE(term_hash_table_check)
// {

//   vector<string> vs;
//   vector<uint64_t> vi;
  
//   readDict("./input", vs);
//   for (vector<string>::const_iterator i=vs.begin(); i!=vs.end(); ++i)
//   {
//     vi.push_back(HashFunction<string>::convert_key(*i));
//   }
//   cout<<"Data is ready!\n";
  
//   struct timeval tvafter,tvpre;
//   struct timezone tz;

//   remove ("./tt");
  
//   {    
//     TermHashTable tb(1000000);
//     gettimeofday (&tvpre , &tz);
//     vector<uint64_t>::const_iterator j=vi.begin();
//     for (vector<string>::const_iterator i=vs.begin(); i!=vs.end(); ++i, ++j)
//     {
//       //cout<<*i<<endl;
//       tb.insert((*i).c_str(), (*i).length(), *j);
//     }
//     gettimeofday (&tvafter , &tz);
//     cout<<"\nAdd into term hash table("<<tb.term_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/1000.<<std::endl;
//     tb.compact();
//     FILE* f = fopen("./tt", "w+");
//     tb.save(f);
//     fclose(f);

//     //cout<<tb;
//     tb.release();
//   }

//   {    
//     FILE* f = fopen("./tt", "r");
//     TermHashTable tb;
//     tb.load(f);

//     gettimeofday (&tvpre , &tz);
//     vector<uint64_t>::const_iterator j=vi.begin();
//     for (vector<string>::const_iterator i=vs.begin(); i!=vs.end(); ++i, ++j)
//     {
//       if (*j != tb.find((*i).c_str(), (*i).length()))
//       {
//         BOOST_CHECK(false);
//         return;
//       }
//     }
//     gettimeofday (&tvafter , &tz);
//     cout<<"\nSearch in dynamic term hash table("<<tb.term_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/1000.<<std::endl;

//     //     char* buf = (char*)malloc(30000000);
//     //     if (tb.load(buf, 30000000, f)==NULL)
//     //     {
//     //       BOOST_CHECK(false);
//     //       return;
//     //     }

//     //     gettimeofday (&tvpre , &tz);
//     //     j=vi.begin();
//     //     for (vector<string>::const_iterator i=vs.begin(); i!=vs.end(); ++i, ++j)
//     //     {
//     //       if (*j != tb.find((*i).c_str(), (*i).length()))
//     //       {
//     //         BOOST_CHECK(false);
//     //         return;
//     //       }
//     //     }
//     //     gettimeofday (&tvafter , &tz);
//     //     cout<<"\nSearch in const term hash table("<<tb.term_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/1000.<<std::endl;

//     //     free(buf);
//   }
  
// }

// BOOST_AUTO_TEST_CASE(value_property_vector_check)
// {
//   struct timeval tvafter,tvpre;
//   struct timezone tz;

//   remove ("./tt");
//   vector<uint64_t> v;

//   {
//     TermPropertyVector<1> tp("./tt");
//     gettimeofday (&tvpre , &tz);
//     for (uint64_t i=0; i<1000000; ++i)
//     {
//       v.push_back(tp.append(i));
//     }
//     gettimeofday (&tvafter , &tz);
//     cout<<"\nInsert into value property vector("<<tp.get_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
//   }

//   {
//     TermPropertyVector<1> tp("./tt");
//     for (uint64_t i=0; i<1000000; ++i)
//     {
//       //std::cout<<v[i]<<std::endl;
//       if(tp.get_freq(v[i]) != i)
//       {
//         cout<<"ERROR: "<<i;
//         BOOST_CHECK(false);
//         return ;
//       }
//     }
//     gettimeofday (&tvafter , &tz);
//     cout<<"\nSearch in value property vector("<<tp.get_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/1000.<<std::endl;
//   }

//   vector<uint64_t> vv;
//   {
//     TermPropertyVector<1> tp("./tt");
//     //cout<<tp.get_num()<<std::endl;
//     for (uint64_t i=0; i<100; ++i)
//     {
//       uint64_t r = rand()%(v.size());
//       vv.push_back(r);
//     }
//     cout<<"Data is ready!\n";
    
//     gettimeofday (&tvpre , &tz);
//     for (uint64_t i=0; i<100; ++i)
//     {
//       tp.set_child(v[vv[i]], 4);
//     }
//     gettimeofday (&tvafter , &tz);
//     cout<<"\nRandom set: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/1000.<<std::endl;
//   }
  
//   {
//     TermPropertyVector<1> tp("./tt");
//     gettimeofday (&tvpre , &tz);
//     for (uint64_t i=0; i<100; ++i)
//     {
//       if(tp.get_child(v[vv[i]])!= 4)
//       {
//         cout<<"ERROR: "<<i;
//         BOOST_CHECK(false);
//         return ;
//       }
//     }
//     gettimeofday (&tvafter , &tz);
//     cout<<"\nRandom get: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/1000.<<std::endl;
//   }
// }


BOOST_AUTO_TEST_CASE(hash_trie_check)
{
  struct timeval tvafter,tvpre;
  struct timezone tz;

  remove ("./tt");
  remove ("./tt.val");
  
  vector<string> vs;

//   {
//     HashTrie<> ht("./tt");
//     vs.push_back("sky");vs.push_back("is");vs.push_back("always");vs.push_back("blue");
//     ht.insert(vs, 3);
//     vs.clear();
  
//     vs.push_back("ocean");vs.push_back("is");vs.push_back("always");vs.push_back("blue");vs.push_back("too");
//     ht.insert(vs, 3);
//     vs.clear();

//     vs.push_back("izenesoft");vs.push_back("is");vs.push_back("full");vs.push_back("of");vs.push_back("software");
//     vs.push_back("engineer");vs.push_back("in");vs.push_back("Shanghai");
//     ht.insert(vs, 3);
//     vs.clear();

//     vs.push_back("welcome");vs.push_back("to");vs.push_back("Shanghai");
//     ht.insert(vs, 3);
//     vs.clear();

//     ht.save();
//   }
//   {
//     HashTrie<> ht("./tt");
    
  
//     vs.push_back("is");
//     BOOST_CHECK(ht.get_freq(vs)==3);
//     vs.clear();

//     vs.push_back("blue");
//     BOOST_CHECK(ht.get_freq(vs)==2);
//     vs.clear();

//     vs.push_back("Shanghai");
//     BOOST_CHECK(ht.get_freq(vs)==2);
//     vs.clear();

//     vs.push_back("is");
//     vs.push_back("always");
//     BOOST_CHECK(ht.get_freq(vs)==2);
//     vs.clear();

//     vs.push_back("ocean");vs.push_back("is");vs.push_back("always");vs.push_back("blue");
//     vs.push_back("too");
//     BOOST_CHECK(ht.get_freq(vs)==1);
//     vs.clear();

//     vector<string> suffix;
//     vector<uint32_t> counts;
//     vs.push_back("is");
//     ht.get_suffix(vs, suffix, counts);
//     BOOST_CHECK(suffix.size()==2);
//     BOOST_CHECK(counts.size()==2);
//     uint32_t t = 0;
//     for (size_t i=0; i<suffix.size();++i)
//     {
//       if (suffix[i].compare("always")==0)
//       {
//         ++t;
//         BOOST_CHECK(counts[i]==2);
//       }
    
//       if (suffix[i].compare("full")==0)
//       {
//         BOOST_CHECK(counts[i]==1);
//         ++t;
//       }
//     }
//     BOOST_CHECK(t==2);
//     vs.clear();

//     vs.push_back("is");vs.push_back("always");
//     ht.get_suffix(vs, suffix, counts);
//     BOOST_CHECK(suffix.size()==1);
//     BOOST_CHECK(counts.size()==1);
//     t = 0;
//     for (size_t i=0; i<suffix.size();++i)
//     {
//       if (suffix[i].compare("blue")==0)
//       {
//         ++t;
//         BOOST_CHECK(counts[i]==2);
//       }
//     }
//     BOOST_CHECK(t==1);
//     vs.clear();
//   }

  remove ("./tt.val");
  remove ("./tt");
  {
    writeDict("./dictionary.txt", vs, 1000000);
    HashTrie<> ht("./tt");

    gettimeofday (&tvpre , &tz);
    for (size_t i=0; i<vs.size()-10; i+=10)
    {
      vector<string> terms;
      for (size_t j=i; j<i+10; ++j)
      {
        //cout<<vs[j]<<" ";
        terms.push_back(vs[j]);
      }
      //cout<<endl;

      ht.insert(terms, 3);
    }
    gettimeofday (&tvafter , &tz);
    cout<<"\nInsert into trie ("<<vs.size()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/1000.<<std::endl;
    cout<<"After insert, node num: "<<ht.node_num()<<endl;
    
    cout<<"Total size: "<<ht.size()<<endl;
    cout<<"After tranverse, node num: "<<ht.node_num()<<endl;
    //ht.save();
  }
  // {
//     cout<<"--------------\n";
//     HashTrie<> ht("./tt");
//     getchar();    
//   }
  
  getchar();
  
}

BOOST_AUTO_TEST_SUITE_END()
