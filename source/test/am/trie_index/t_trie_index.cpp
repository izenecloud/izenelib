
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
#include <am/trie_index/term_hash_table.hpp>
#include <am/trie_index/hash_trie.hpp>
#include <am/trie_index/doc_list.hpp>
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

BOOST_AUTO_TEST_CASE(doc_list_check)
{
  {
    FILE* f = fopen("./tt", "w+");
    DocList<> list(0);
  
    for (uint32_t i=1; i<1000000; ++i)
    {
      list.append(i);
    }
  
    cout<<"1M 32bit integers takes "<<list.size()<<" bytes.\n";
    list.compact();
    cout<<"After compacting, it's "<<list.size()<<" bytes.\n";

    for (uint32_t i=0; i<1000000; ++i)
    {
      if (list[i]!=i)
      {
        cout<<"DocList ERROR: "<<i<<endl;
        BOOST_CHECK(false);
        break;
      }
    }
    
    list.save(f);
    fclose(f);
  }

  {
    FILE* f = fopen("./tt", "r+");
    DocList<> list(1000000);
  
    for (uint32_t i=1; i<100; ++i)
    {
      list.append(1000000+i);
    }

    DocList<> list1(f, 0);
    
    for (uint32_t i=0; i<1000000; ++i)
    {
      if (list1[i]!=i)
      {
        cout<<"DocList ERROR: "<<i<<endl;
        BOOST_CHECK(false);
        break;
      }
    }
    
    list1.append(list);
    
    for (uint32_t i=0; i<1000000+100; ++i)
    {
      if (list1[i]!=i)
      {
        cout<<"DocList ERROR: "<<list1[i]<<" "<<i<<endl;
        BOOST_CHECK(false);
        break;
      }
    }

  }
  
}

BOOST_AUTO_TEST_CASE(term_hash_table_check)
{
  const uint32_t SIZE=1000000;
  vector<uint64_t> vi;
  
  for (uint64_t i=0; i<SIZE; ++i)
  {
    vi.push_back(rand());
  }
  cout<<"Data is ready!\n";
  
  struct timeval tvafter,tvpre;
  struct timezone tz;

  remove ("./tt");
  
  {    
    TermHashTable tb(1000000);
    bool is_new = false;
    gettimeofday (&tvpre , &tz);
    for (vector<uint64_t>::const_iterator i=vi.begin(); i!=vi.end(); ++i)
    {
      //cout<<*i<<endl;
      Term t(tb.insert(*i, is_new));
      if (is_new)
        t.set_doc_list(*i/2);
    }
    gettimeofday (&tvafter , &tz);
    cout<<"\nAdd into term hash table("<<tb.term_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/1000.<<std::endl;
    tb.compact();
    FILE* f = fopen("./tt", "w+");
    tb.save(f);
    fclose(f);

    //cout<<tb;
    tb.release();
  }

  {    
    FILE* f = fopen("./tt", "r");
    TermHashTable tb;
    tb.load(f);

    gettimeofday (&tvpre , &tz);
    for (vector<uint64_t>::const_iterator i=vi.begin(); i!=vi.end(); ++i)
    {
      Term t(tb.find(*i));
      if (t.is_null() || t.get_doc_list()!=(*i)/2)
      {
        BOOST_CHECK(false);
        return;
      }
    }
    gettimeofday (&tvafter , &tz);
    cout<<"\nSearch in dynamic term hash table("<<tb.term_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/1000.<<std::endl;

    //     char* buf = (char*)malloc(30000000);
    //     if (tb.load(buf, 30000000, f)==NULL)
    //     {
    //       BOOST_CHECK(false);
    //       return;
    //     }

    //     gettimeofday (&tvpre , &tz);
    //     j=vi.begin();
    //     for (vector<string>::const_iterator i=vs.begin(); i!=vs.end(); ++i, ++j)
    //     {
    //       if (*j != tb.find((*i).c_str(), (*i).length()))
    //       {
    //         BOOST_CHECK(false);
    //         return;
    //       }
    //     }
    //     gettimeofday (&tvafter , &tz);
    //     cout<<"\nSearch in const term hash table("<<tb.term_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/1000.<<std::endl;

    //     free(buf);
  }
  
}

BOOST_AUTO_TEST_CASE(hash_trie_check)
{
  struct timeval tvafter,tvpre;
  struct timezone tz;

  remove ("./tt");
  remove ("./tt.doc");
  
  vector<uint64_t> vs;

  {
    HashTrie<> ht("./tt");
    vs.push_back(1);vs.push_back(2);vs.push_back(3);vs.push_back(4);
    ht.insert(vs, 1);
    vs.clear();
  
    vs.push_back(5);vs.push_back(2);vs.push_back(3);vs.push_back(4);vs.push_back(6);
    ht.insert(vs, 1);
    vs.clear();

    vs.push_back(7);vs.push_back(2);vs.push_back(8);vs.push_back(9);vs.push_back(10);
    vs.push_back(11);vs.push_back(12);vs.push_back(13);
    ht.insert(vs, 2);
    vs.clear();

    vs.push_back(14);vs.push_back(15);vs.push_back(13);
    ht.insert(vs, 3);
    vs.clear();

    vector<uint32_t> docs;
    vs.push_back(2);
    BOOST_CHECK(ht.get_freq(vs)==3);
    ht.get_docs(vs, docs);
    BOOST_CHECK(docs.size()==2);
    BOOST_CHECK(docs[0]==1);
    BOOST_CHECK(docs[1]==2);
    vs.clear();
    
    ht.save();
  }
  {
    HashTrie<> ht("./tt");
    vector<uint32_t> docs;
  
    vs.push_back(2);
    BOOST_CHECK(ht.get_freq(vs)==3);
    ht.get_docs(vs, docs);
    BOOST_CHECK(docs.size()==2);
    BOOST_CHECK(docs[0]==1);
    BOOST_CHECK(docs[1]==2);
    vs.clear();

    vs.push_back(4);
    BOOST_CHECK(ht.get_freq(vs)==2);
    vs.clear();

    vs.push_back(13);
    BOOST_CHECK(ht.get_freq(vs)==2);
    vs.clear();

    vs.push_back(2);
    vs.push_back(3);
    BOOST_CHECK(ht.get_freq(vs)==2);
    vs.clear();

    vs.push_back(5);vs.push_back(2);vs.push_back(3);vs.push_back(4);
    vs.push_back(6);
    BOOST_CHECK(ht.get_freq(vs)==1);
    vs.clear();

    vector<uint64_t> suffix;
    vector<uint32_t> counts;
    vs.push_back(2);
    ht.get_suffix(vs, suffix, counts);
    BOOST_CHECK(suffix.size()==2);
    BOOST_CHECK(counts.size()==2);
    uint32_t t = 0;
    for (size_t i=0; i<suffix.size();++i)
    {
      if (suffix[i] == 3)
      {
        ++t;
        BOOST_CHECK(counts[i]==2);
      }
    
      if (suffix[i]==8)
      {
        BOOST_CHECK(counts[i]==1);
        ++t;
      }
    }
    BOOST_CHECK(t==2);
    vs.clear();

    vs.push_back(2);vs.push_back(3);
    ht.get_suffix(vs, suffix, counts);
    BOOST_CHECK(suffix.size()==1);
    BOOST_CHECK(counts.size()==1);
    t = 0;
    for (size_t i=0; i<suffix.size();++i)
    {
      if (suffix[i]==4)
      {
        ++t;
        BOOST_CHECK(counts[i]==2);
      }
    }
    BOOST_CHECK(t==1);
    vs.clear();
  }

  remove ("./tt.val");
  remove ("./tt");
  {
    const uint32_t SIZE = 5000000;
    for (uint64_t i=0; i<SIZE; ++i)
      vs.push_back(rand()%80000);
    cout<<"Data is ready!\n";

    HashTrie<> ht("./tt");

    uint32_t docid = 0;
    gettimeofday (&tvpre , &tz);
    for (size_t i=0; i<vs.size()-10; i+=10)
    {
      vector<uint64_t> terms;
      for (size_t j=i; j<i+10; ++j)
      {
        //cout<<vs[j]<<" ";
        terms.push_back(vs[j]);
      }
      //cout<<endl;

      if (i%(100*10)==0)
        ++docid;

      ht.insert(terms, docid);
    }
    gettimeofday (&tvafter , &tz);
    cout<<"\nInsert into trie ("<<vs.size()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/1000.<<std::endl;
    cout<<"After insert, node num: "<<ht.node_num()<<endl;
    
    cout<<"Total size: "<<ht.size()<<endl;
    cout<<"After tranverse, node num: "<<ht.node_num()<<endl;
    //ht.save();
    getchar();
  }
  // {
  //     cout<<"--------------\n";
  //     HashTrie<> ht("./tt");
  //     getchar();    
  //   }
  getchar();
}

BOOST_AUTO_TEST_SUITE_END()
