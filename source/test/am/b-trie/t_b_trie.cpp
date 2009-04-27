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

#include <util/log.h>

#include <string>
#include <time.h>
#include <math.h>
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <iostream>

#include <am/trie/b_trie.hpp>
#include <am/trie/alphabet.hpp>
#include <am/map/map.hpp>
#include <signal.h>

#include <wiselib/ustring/UString.h>


USING_IZENE_LOG();

using namespace wiselib;
using namespace izenelib::am;
using namespace std;

#define SIZE 27
#define ENCODE_TYPE UString::UTF_8//EUC_KR//GB2312//

BOOST_AUTO_TEST_SUITE( b_trie_suite )
//extern int debug_count;

typedef string string_type;

void checkInput(const string_type& u)
{
  if (u.length() == 0)
    cout<<"Empty string!\n";
  
  for (size_t i=0; i<u.length(); i++)
  {
    if (AlphabetNode<string_type::value_type>::getIndexOf(u[i])>=a2z_size)
    {
      cout<<"Not exist in alphabet: "<<u[i]<<"   ";
      //u.displayStringValue(ENCODE_TYPE, cout);
      cout<<endl;
      break;
    }
    
  }
}

UString init (const string& str, UString*)
{
  UString t(str.c_str(), ENCODE_TYPE);
  return t;
}

string init (const string& str, string*)
{
  return str;
}

void readDict(const string& dict, vector<string_type>& v)
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

      string_type str  = init(bf.substr(start, i-start), (string_type*)0);
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



/**
 *This is for testing the correction of insertion and querying of Ustring version B-trie.
 *
 **/
BOOST_AUTO_TEST_CASE(B_trie_regex_check )
{
  using namespace wiselib;
  //signal(SIGSEGV, &dump);

  remove("./test.buk");
  remove("./test.nod");
  remove("./test.has");
  
  vector<string_type> vstr;
  vector<string_type*> vp;

  readDict("./input", vstr);
  cout<<"\nData is ready!\n";
  clock_t start, finish;
   
  BTrie<string_type> trie("./test");
  
  Map<string, uint64_t, 10> map;

  
  
  for (vector<string_type>::iterator i=vstr.begin(); i!=vstr.end();i++)
  {
    string_type* str = new string_type (*i);

//     string s(str->c_str(), str->size());
//     map.insert(s, rand());
    
    vp.push_back(str);
  }
  //map.save("hehe");
  //map.load("hehe");
  
  start = clock();
  for (vector<string_type*>::iterator i=vp.begin(); i!=vp.end();i++,debug_count++)
  {
//     (*i)->displayStringValue(ENCODE_TYPE);
//     cout<<endl;
    
    //uint64_t t = rand();
    trie.insert(*(*i),22);
    
    //if (t!=trie.find(*(*i)))
    {
//       (*i)->displayStringValue(ENCODE_TYPE);
//       cout<<endl<<debug_count<<endl<<trie.find(*(*i))<<endl;
      
      //break;
    }
    
  }
  trie.flush();
  finish = clock();
  printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, vp.size());
  cout<<"Node amount: "<<trie.getNodeAmount()<<endl;
  //------------------------------------------------------------------------------

//   //trie.display(cout, "far");
//   //BTrie<ENCODE_TYPE> trie("./test");  
//   start = clock();
//   uint64_t diskAdd = 1;
//   uint32_t memAdd = 0;
//   vector<item_pair<string_type> > ip;
//   //*1M77N?fH*C3lJt
//   //cout<<trie.query(string_type("GIdku3vr_C5jIUtVQHXNf5pPWRn3uiJa", ENCODE_TYPE))<<endl;
  
//   trie.findRegExp(string_type("*u3vr_C5jIUtVQHXNf?pPW*iJa", ENCODE_TYPE), ip);//(memAdd, diskAdd, , string_type(), ip);
//   for (vector<item_pair<string_type> >::iterator i = ip.begin(); i!= ip.end(); i++)
//   {
//     (*i).str_.displayStringValue(ENCODE_TYPE, cout);
//     cout<<"===>"<<(*i).addr_<<endl;
//   }
  

  
//   finish = clock();
//   printf( "\nIt takes %f seconds to find!\n", (double)(finish - start)/CLOCKS_PER_SEC);

//   BTrie<wiselib::string_type> trie("./test"); 
//     ifstream inf("./dict1.txt");
//     string str;
//     unsigned int id = 0;
//     while (inf>>str)
//     {
//         string_type ustr(str,string_type::UTF_8);
//         trie.insert(ustr,2);
//         //cout<<str;
        
//         str.clear();
//     }
// 	trie.flush();
    

  BTrie<string_type> trie1("./test");
  vector<item_pair<string_type> > ip;
//     vector<uint64_t> ip;xknajiqdasotghofnza
  string_type pattern = init("weyzrcpe*gkhzwvb", (string_type*)0);
 	trie1.findRegExp(pattern, ip);
    
 	for (vector<item_pair<string_type> >::iterator i = ip.begin(); i!= ip.end(); i++)
 	{
      //(*i).str_.displayStringValue(string_type::UTF_8, cout);
      //cout<<"===>"<<(*i)<<endl;
      cout<<"pppppp\n";
      
    }

}
BOOST_AUTO_TEST_SUITE_END()


