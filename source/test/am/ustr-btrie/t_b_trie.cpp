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
#include <am/ustr-btrie/bucket.hpp>
#include <am/ustr-btrie/alphabet_node.hpp>
#include <am/ustr-btrie/bucket_cache.hpp>
#include <am/ustr-btrie/alphabet.hpp>
#include <string>
#include <time.h>
#include <math.h>
#include <boost/test/unit_test.hpp>
#include <time.h>
#include <util/log.h>
#include <stdio.h>
#include <list>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <am/ustr-btrie/node_cache.hpp>
#include <am/ustr-btrie/b_trie.hpp>
#include <am/ustr-btrie/alphabet.hpp>
#include <signal.h>
//#include <am/ustr-btrie/automation.hpp>
#include <ustring/UString.h>
#include <am/linear_hash_table/linearHashTable.hpp>

using namespace sf1lib;
using namespace izenelib::am;
using namespace std;

#define SIZE 27
#define ENCODE_TYPE UString::UTF_8//EUC_KR//GB2312//

USING_IZENE_LOG();

BOOST_AUTO_TEST_SUITE( b_trie_suite )
//extern int debug_count;

void checkInput(const UString& u)
{
  if (u.length() == 0)
    cout<<"Empty string!\n";
  
  for (size_t i=0; i<u.length(); i++)
  {
    if (AlphabetNode<>::getIndexOf(u[i])>=a2z_size)
    {
      u.displayStringValue(ENCODE_TYPE, cout);
      cout<<endl;
      break;
    }
    
  }
}

void readDict(const string& dict, vector<UString>& v)
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
  string bf(buffer, length);
  //UString bf(tmp, ENCODE_TYPE);
  size_t i=0;
  size_t start = 0;
  
  for (; i<bf.length(); i++)
  {
    if (bf[i]==' '|| bf[i]=='\n')
    {
        
      if(start+1 == i && (bf[start]==' '||bf[start]=='\n'))
      {
        start = i+1;
        continue;
      }

      UString str(bf.substr(start, i-start), ENCODE_TYPE);
      //UString tp;
      //bf.subString(tp ,start, i-start);
      checkInput(str);
      v.push_back(str);
      // str.displayStringValue(ENCODE_TYPE, cout);
//       cout<<endl;
      
      start = i+1;
      i++;
                
    }
  }

  //cout<<dicTermStr_;
  
  delete buffer;
  
}

void dump(int signo)
{
        char buf[1024];
        char cmd[1024];
        FILE *fh;

        snprintf(buf, sizeof(buf), "/proc/%d/cmdline", getpid());
        if(!(fh = fopen(buf, "r")))
                exit(0);
        if(!fgets(buf, sizeof(buf), fh))
                exit(0);
        fclose(fh);
        if(buf[strlen(buf) - 1] == '\n')
                buf[strlen(buf) - 1] = '\0';
        snprintf(cmd, sizeof(cmd), "gdb %s %d", buf, getpid());
        system(cmd);

        exit(0);
}

BOOST_AUTO_TEST_CASE(B_trie_insertion_check )
{
  signal(SIGSEGV, &dump);
    
  cout<<"Running B_trie_insertion_check ... \n";
  
//   AlphabetGenerator<ENCODE_TYPE> alp("abcdefghijklmnopqrstuvwxyz");
  
//   cout<<alp;
  
  remove("./test.buk");
  remove("./test.nod");
  remove("./test.has");
  vector<UString> vstr;
  vector<UString*> vp;
  readDict("./input", vstr);
  cout<<"\nData is ready!\n";
  
  BTrie<ENCODE_TYPE> trie("./test");

  clock_t start, finish;
  for (vector<UString>::iterator i=vstr.begin(); i!=vstr.end();i++)
  {
    //transform((*i).begin(), (*i).end(), (*i).begin(),::tolower);
    //cout<<*i<<endl;
    UString* str = new UString (*i);
    //str->displayStringValue(ENCODE_TYPE, cout);
    
    vp.push_back(str);
  }

  
  start = clock();
  for (vector<UString*>::iterator i=vp.begin(); i!=vp.end();i++,debug_count++)
  {
    //cout<<c<<"  "<<trie.getNodeAmount()<<"  ";
    //(*i)->displayStringValue(ENCODE_TYPE, cout);
    //cout<<debug_count<<endl;
    trie.insert(*i,2);
  }
  

  trie.flush();
  //trie.flush();
  finish = clock();
  printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, vp.size());
  //trie.display(cout, "far");
  cout<<"Node amount: "<<trie.getNodeAmount()<<endl;
  
  start = clock();
  for (vector<UString>::iterator i=vstr.begin(); i!=vstr.end();i++)
  {
    //cout<<*i<<endl;
    if (trie.query(*i) != 2)
    {
      (*i).displayStringValue(ENCODE_TYPE, cout);
      cout<<" Not Found!\n";
      break;
    }
    
  }

  //trie.display(cout,UString("澎纲测赂梯坏嘘确扣", ENCODE_TYPE));
  
  finish = clock();
  printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start)/CLOCKS_PER_SEC, vstr.size());

}

BOOST_AUTO_TEST_SUITE_END()


