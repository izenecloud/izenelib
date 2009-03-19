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
#define IZENE_LOG

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
//#include <util/log.h>
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
USING_IZENE_LOG();

using namespace sf1lib;
using namespace izenelib::am;
using namespace std;

#define SIZE 27
#define ENCODE_TYPE UString::UTF_8//EUC_KR//GB2312//



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
      cout<<"Not exist in alphabet: "<<u[i]<<"   ";
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

/**
 *This is for testing the correction of insertion and querying of Ustring version B-trie.
 *
 **/

// BOOST_AUTO_TEST_CASE(B_trie_insertion_check )
// {
//   signal(SIGSEGV, &dump);
    
//   LDBG_<<"Running B_trie_insertion_check ... \n";
  
  //  AlphabetGenerator<ENCODE_TYPE> alp("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890");
  
  //  cout<<alp;
  
//   remove("./test.buk");
//   remove("./test.nod");
//   remove("./test.has");
  
//   vector<UString> vstr;
//   vector<UString*> vp;
//   readDict("./dict1.txt", vstr);
//   cout<<"\nData is ready!\n";
  
//   BTrie<ENCODE_TYPE> trie("./test");

//     clock_t start, finish;
//   for (vector<UString>::iterator i=vstr.begin(); i!=vstr.end();i++)
//   {
//     //transform((*i).begin(), (*i).end(), (*i).begin(),::tolower);
//     //cout<<*i<<endl;
//     UString* str = new UString (*i);
//     //str->displayStringValue(ENCODE_TYPE, cout);
    
//     vp.push_back(str);
//   }

  
//   start = clock();
//   for (vector<UString*>::iterator i=vp.begin(); i!=vp.end();i++,debug_count++)
//   {
//     trie.insert(*i,2);
//   }
  

//   trie.flush();
//   //trie.flush();
//   finish = clock();
//   printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, vp.size());
//   //trie.display(cout, "far");
//   cout<<"Node amount: "<<trie.getNodeAmount()<<endl;

//   BTrie<ENCODE_TYPE> trie("./test");
//   cout<<"Node amount: "<<trie.getNodeAmount()<<endl;
//   cout<<"Loaded.\n";
//   debug_count = 100;
//   start = clock();
//   for (vector<UString>::iterator i=vstr.begin(); i!=vstr.end();i++)
//   {
//     if (trie.query(*i) != 2)
//     {
//       (*i).displayStringValue(ENCODE_TYPE, cout);
//       cout<<" Not Found!\n";
//       break;
//     }
    
//   }

  //trie1.display(cout,UString("GIdku3vr_C5jIUtVQHXNf5pPWRn3uiJa", ENCODE_TYPE));
  
//   finish = clock();
//   printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start)/CLOCKS_PER_SEC, vstr.size());

// }

// BOOST_AUTO_TEST_SUITE_END()


/**
 *This is for testing the correction of insertion and querying of Ustring version B-trie.
 *
 **/
BOOST_AUTO_TEST_CASE(B_trie_regex_check )
{
  signal(SIGSEGV, &dump);

  remove("./test.buk");
  remove("./test.nod");
  remove("./test.has");
  
//   vector<UString> vstr;
//   vector<UString*> vp;
//   readDict("./dict1.txt", vstr);
//   cout<<"\nData is ready!\n";
//    clock_t start, finish;
   
//   BTrie<ENCODE_TYPE> trie("./test");


//   for (vector<UString>::iterator i=vstr.begin(); i!=vstr.end();i++)
//   {
//     UString* str = new UString (*i);
    
//     vp.push_back(str);
//   }
//   start = clock();
//   for (vector<UString*>::iterator i=vp.begin(); i!=vp.end();i++,debug_count++)
//   {
//     trie.insert(*i,2);
//   }
//   trie.flush();
//   finish = clock();
//   printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, vp.size());
//   cout<<"Node amount: "<<trie.getNodeAmount()<<endl;
//   //------------------------------------------------------------------------------

//   //trie.display(cout, "far");
//   //BTrie<ENCODE_TYPE> trie("./test");  
//   start = clock();
//   uint64_t diskAdd = 1;
//   uint32_t memAdd = 0;
//   vector<item_pair> ip;
//   //*1M77N?fH*C3lJt
//   //cout<<trie.query(UString("GIdku3vr_C5jIUtVQHXNf5pPWRn3uiJa", ENCODE_TYPE))<<endl;
  
//   trie.findRegExp(UString("*u3vr_C5jIUtVQHXNf?pPW*iJa", ENCODE_TYPE), ip);//(memAdd, diskAdd, , UString(), ip);
//   for (vector<item_pair>::iterator i = ip.begin(); i!= ip.end(); i++)
//   {
//     (*i).str_.displayStringValue(ENCODE_TYPE, cout);
//     cout<<"===>"<<(*i).addr_<<endl;
//   }
  

  
//   finish = clock();
//   printf( "\nIt takes %f seconds to find!\n", (double)(finish - start)/CLOCKS_PER_SEC);

  BTrie<sf1lib::UString::UTF_8> trie("./test"); 
    ifstream inf("./dict1.txt");
    string str;
    unsigned int id = 0;
    while (inf>>str)
    {
        sf1lib::UString ustr(str,sf1lib::UString::UTF_8);
        trie.insert(ustr,2);
        //cout<<str;
        
        str.clear();
    }
	trie.flush();
    
    //vector<item_pair> ip;
    vector<uint64_t> ip;
	UString pattern("*u3vr_C5jIUtVQHXNf?pPW*iJa", sf1lib::UString::UTF_8);
	trie.findRegExp(pattern, ip);
    
	for (vector<uint64_t>::iterator i = ip.begin(); i!= ip.end(); i++)
	{
	 // (*i).str_.displayStringValue(sf1lib::UString::UTF_8, cout);
	  cout<<"===>"<<(*i)<<endl;
	}

}
BOOST_AUTO_TEST_SUITE_END()


