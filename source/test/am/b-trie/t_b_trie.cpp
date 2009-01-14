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
#include <am/trie/alphabet_node.hpp>
#include <am/trie/bucket.hpp>
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

#define SIZE 27
using namespace izenelib::am;
using namespace std;

BOOST_AUTO_TEST_SUITE( b_trie_suite )

BOOST_AUTO_TEST_CASE(AlphabetNode_check )
{
  USING_IZENE_LOG();
  remove("./test");
  FILE* f = fopen("./test", "w+");
  AlphabetNode<>* alph = new AlphabetNode<>(f);
  uint64_t root = alph->add2disk();
  cout<<"root:"<<(int)root<<endl;
  list<AlphabetNode<>*> v;
  v.push_back(alph);

  clock_t start, finish;
  uint64_t c = 1;
  start = clock();
  while (v.size()!=0)
  {
    if (c < SIZE)
    {
      for (int i=0; i<alph->getSize(); i++)
      {
        AlphabetNode<>* n = new AlphabetNode<>(f);
        v.push_back(n);
        v.front()->setDiskAddr(i, n->add2disk());
        c++;
      }
    }
    
    v.front()->update2disk();
    //cout<<*(v.front());
    delete v.front();
    v.pop_front();
  }
  finish = clock();
  fflush(f);
  
  start = clock();
  uint64_t addr = 1;
  while(c!=0)
  {
    c--;
    AlphabetNode<> n(f);
    if (!n.load(addr))
    {
      cout<<"Wrong loading!\n";
      continue;
    }

    n.setDiskAddr(3, 1234);
    if (!n.update2disk())
    {
      cout<<"Wrong updating!\n";
      continue;
    }
    
    if (!n.load(addr))
    {
      cout<<"Wrong loading!\n";
      continue;
    }
    addr += 216;
    //cout<<n<<endl;
    
  }
  finish = clock();
  printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, SIZE);
  fclose(f);
  
  //  tb.display(std::cout);
   
}
string genRandomStr(unsigned int maxLen)
{
  string s;
  maxLen = rand()%maxLen;
  for (unsigned int i=0; i<maxLen; i++)
    s += 'a'+rand()%26;

  return s;
}

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

BOOST_AUTO_TEST_CASE(Bucket_add_string )
{
  //unsigned int amount =  10000;
  //unsigned int maxLen = 10;
  
  USING_IZENE_LOG();
  remove("./test");
  FILE* f = fopen("./test", "w+");
  Bucket<> b(f);
  Bucket<> b2(f);
  
  clock_t start, finish;

  vector<string> v;
  readDict("./dict", v);
  start = clock();

  unsigned int k = 0;
  unsigned int size = 0;
  
  for (vector<string>::iterator i=v.begin(); i!=v.end();i++)
  {
    transform((*i).begin(), (*i).end(), (*i).begin(),::tolower);
    unsigned int p=b.addString(*i,k);
    if(k == p)
    {
      b.split(&b2);
      break;
    }
    
    k = p;

    size+=(*i).length()+sizeof(uint64_t)+sizeof(uint32_t);
    //cout<<p<<" "<<*i<<endl;
  }
  finish = clock();
  cout<<b;
  cout<<b2;
  cout<<"\nSize: "<<size+sizeof(uint8_t)*2+sizeof(uint32_t)*2;
  
  printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, k);
  fclose(f);
  
  //  tb.display(std::cout);
   
}

BOOST_AUTO_TEST_SUITE_END()

