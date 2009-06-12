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

//#define HAVE_USR_INCLUDE_MALLOC_H

//#include <am/istring/small_obj_alloc.hpp>

#include <time.h>

#include <boost/test/unit_test.hpp>
#include <assert.h>
#include <iostream>
#include <util/istring/vector_string.hpp>
#include <util/istring/deque_string.hpp>
#include <vector>
#include <string>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <fstream>
#include <util/istring/istring>
#include <util/istring/algo>

using namespace std;
using namespace izenelib::util;

BOOST_AUTO_TEST_SUITE( t_vstring_suite )

#define SCALE 10000000
#define MAX_LEN 1024
typedef char CharT;
typedef vector_string<CharT> istring;
//typedef istring istring;

size_t getLen(const char* s)
{
  char e = '\0';
  size_t i = 0;
    
  while (s[i]!=e)
  {
    i++;
  }

  return i;
}

CharT* conv(const char* ch, CharT* buf)
{
  size_t len = getLen(ch);
  for (size_t i=0; i<len; i++)
    buf[i] = (CharT)ch[i];

  buf[len] = '\0';
  
  return buf;
}

CharT buf[256];

BOOST_AUTO_TEST_CASE(izene_istring_construction_check)
{
  int t = 0;
  istring s = conv("asasasas", buf);
  istring s1= s;

  if (s1.compare(s)!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  istring s2(s, 2, 3);
  if (s2.compare(conv("asa",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  istring s3(s, 2);
  if (s3.compare(conv("asasas",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  istring s4(conv("sdfsfsfsdf",buf), 5);
  if (s4.compare(conv("sdfsf",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  istring s5(5, 'd');
  if (s5.compare(conv("ddddd",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  string st;
  st.assign((char*)conv("asasas",buf), 6*sizeof(CharT));
  istring s6(st);
  if (s6.compare(conv("asasas",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  vector<CharT> vt;
  vt.push_back('g');
  vt.push_back('f');
  vt.push_back('v');
  vt.push_back('d');
  vt.push_back('s');
  vt.push_back('w');
  
  istring s7(vt);
  if (s7.compare(conv("gfvdsw",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  cout<<"izene_istring_construction_check is done!\n";
}

BOOST_AUTO_TEST_CASE(izene_istring_capacity_check)
{
  int t=0;
  istring is(conv("dkfgjhghdkslsdjk",buf));
  istring s = is;

  if (s.length()!=16)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  if (s.size() != 16*sizeof(CharT))
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s.resize(32);
  if (s.length()!= 32 || s.max_size()!=33)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = is;
  is.resize(10);
  if (is.capacity()!=11*sizeof(CharT) || is.max_size()!=11)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = is;
  is.reserve(11);
  if (is.capacity()!=11*sizeof(CharT) || is.max_size()!=11)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = is;
  s.resize(32, 'T');
  if (s.length()!=32 || s.max_size()!=33)
    cout<<"["<<t<<"] ERROR\n";

  cout<<"izene_istring_capacity_check is done!\n";
}

BOOST_AUTO_TEST_CASE(izene_istring_element_access_check)
{
  istring s(conv("sfdfsfssfssfs",buf));
  istring ss = s;
  int t= 0;

  if (ss[2] != 'd')
    cout<<"["<<t<<"] ERROR\n";

  t++;
  if (s[3] != 'f')
    cout<<"["<<t<<"] ERROR\n";

  t++;
  if (ss.at(2) != 'd')
    cout<<"["<<t<<"] ERROR\n";

  t++;
  if (ss.at(3) != 'f')
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss[2] = 'k';
  if (ss.at(2) != 'k')
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s[3] = 'h';
  if (s.at(3) != 'h')
    cout<<"["<<t<<"] ERROR\n";

  cout<<"izene_istring_element_access_check is done!\n";
}

BOOST_AUTO_TEST_CASE(izene_istring_modifiers_check)
{
  istring s(conv("sfdfsfssfssfs",buf));
  istring ss = s;
  int t= 0;

  ss += s;
  if (ss.compare(conv("sfdfsfssfssfssfdfsfssfssfs",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = s;
  ss += conv("TTTT",buf);
  if (ss.compare(conv("sfdfsfssfssfsTTTT",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = s;
  ss += 'T';
  if (ss.compare(conv("sfdfsfssfssfsT",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
    
  t++;
  s += conv("kk",buf);
  ss = s;
  ss.append(ss, 2, 3);
  if (ss.compare(conv("sfdfsfssfssfskkdfs",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = ss;
  s.append(conv("oweirer",buf), 3);
  if (s.compare(conv("sfdfsfssfssfskkdfsowe",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = ss;
  s.append(4,' ');
  if (s.compare(conv("sfdfsfssfssfskkdfs    ",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = ss;
  s.append(ss, 6);
  if (s.compare(conv("sfdfsfssfssfskkdfsssfssfskkdfs",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = ss;
  s.push_back('o');
  s.push_back('A'); 
  if (s.compare(conv("sfdfsfssfssfskkdfsoA",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = conv("fdfuiehjka",buf);
  ss = s;
  ss.insert(3, s);
  if (ss.compare(conv("fdfufdfuiehjkaiehjka",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = conv("fdfuiehjka",buf);
  ss = s;
  s.insert(3, s);
  if (s.compare(conv("fdfufdfuiehjkaiehjka",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  //------10
  t++;
  s = conv("fdfuiehjka",buf);
  ss = s;
  ss.insert(3, conv("dsfsfs",buf));
  if (ss.compare(conv("fdfudsfsfsiehjka",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = conv("fdfuiehjka",buf);
  ss = s;
  ss.insert(3, conv("dsfsfs",buf), 3);
  if (ss.compare(conv("fdfudsfiehjka",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = conv("fdfuiehjka",buf);
  ss = s;
  ss.erase(3);
  if (ss.compare(conv("fdf",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = conv("fdfuiehjka",buf);
  ss = s;
  ss.erase(3, 3);
  if (ss.compare(conv("fdfhjka",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = conv("fdfuiehjka",buf);
  s.erase(3);
  if (s.compare(conv("fdf",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  // 15
  t++;
  s = conv("fdfuiehjka",buf);
  s.erase(3, 3);
  if (s.compare(conv("fdfhjka",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = conv("fdfuiehjka",buf);
  ss = conv("jjj",buf);
  s.replace(3, 3, ss);
  if (s.compare(conv("fdfjjjhjka",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = conv("fdfuiehjka",buf);
  istring sss = s;
  ss = conv("jjj",buf);
  sss.replace(3, 3, ss);
  if (sss.compare(conv("fdfjjjhjka",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = conv("fdfuiehjka",buf);
  ss = s;
  CharT j[3];
  ss.copy(j, 3, 3);
  istring ssss(j, 3);
  if (ssss.compare(conv("uie",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = conv("fdfuiehjka",buf);
  ss = conv("sss",buf);
  ss.swap(s);
  if (ss.compare(conv("fdfuiehjka",buf))!=0 || s.compare(conv("sss",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  // 20  
  t++;
  CharT* jj = conv("fdfuiehjka",buf);
  ss.attach(jj);
  if (ss.compare(conv("fdfuiehjka",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss.attach(jj, 3);
  if (ss.compare(conv("fdf",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  cout<<"izene_istring_modifiers_check is done!\n";
}

BOOST_AUTO_TEST_CASE(izene_istring_str_operation_check)
{
  istring s = conv("sdjkfjkdsjkfjkshkla",buf);
  istring ss = conv("jkfj",buf);
  int t=0;
  
  if (s.find(ss) != 2)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  if (s.find(ss, 3) != 9)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  if (s.rfind(ss) != 12)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  if (s.rfind(ss, 9) != 5)
    cout<<"["<<t<<"] ERROR\n";

  cout<<"izene_istring_str_operation_check is done!\n";
}

BOOST_AUTO_TEST_CASE(izene_istring_serialization_check)
{
  typedef boost::archive::binary_iarchive iarchive;
  typedef boost::archive::binary_oarchive oarchive;
 
  istring s(conv("ssdkfsdjghnkx",buf));
  ofstream of("./db", ios_base::trunc);
  oarchive oa(of);
  oa<<s;
  of.close();
  
  istring is;
  ifstream ifs("./db");
  iarchive ia(ifs);
  ia >> is;

  int t=0;
  if (s.compare(is) != 0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ifstream ifs1("./db");
  iarchive ia1(ifs1);
  ia1 >> s;
  if (s.compare(is) != 0)
    cout<<"["<<t<<"] ERROR\n";

  cout<<"izene_istring_serialization_check is done!\n";
}

BOOST_AUTO_TEST_CASE(izene_istring_iterator_check)
{
  vector<CharT> vt;
  vt.push_back('g');
  vt.push_back('f');
  vt.push_back('v');
  vt.push_back('d');
  vt.push_back('s');
  vt.push_back('w');

  istring s;
  s.assign<vector<CharT>::iterator >(vt.begin(),vt.end());

  int t=0;
  if (s.compare(conv("gfvdsw",buf)) != 0)
    cout<<"["<<t<<"] ERROR\n";

  s.insert(s.begin(), vt.begin(),vt.end());
  t++;
  if (s.compare(conv("ggfvdswfvdsw",buf)) != 0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = conv("dkghk",buf);
  istring ss;
  for (istring::reverse_iterator i=s.rbegin(); i<s.rend(); i++)
  {
    ss += *i;
  }
  if (ss.compare(conv("khgkd",buf)) != 0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("",buf);
  for (istring::const_iterator i=s.begin(); i<s.end(); i++)
  {
    ss += *i;
  }
  if (ss.compare(conv("dkghk",buf)) != 0)
    cout<<"["<<t<<"] ERROR\n";
  
  cout<<"izene_istring_iterator_check is done!\n";
}

BOOST_AUTO_TEST_CASE(izene_istring_algorithms_check)
{

  typedef Algorithm<istring> algo;
  //typedef ialgo algo;
  
  int t=0;
  
  istring s(conv("-11013",buf));
  if (algo::to_integer(s)!= -11013)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = conv("11234",buf);
  if (algo::to_integer(s)!= 11234)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = conv("-11013",buf);
  if (algo::to_long(s)!= -11013)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = conv("11234",buf);
  if (algo::to_long(s)!= 11234)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = conv("11234",buf);
  if (algo::to_ulong(s)!= 11234)
    cout<<"["<<t<<"] ERROR\n";

  // 5  
  t++;
  s = conv("1.1234",buf);
  if ((int)(algo::to_float(s)/ 1.1234 +.1) !=1)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = conv("11234.",buf);
  if (algo::to_float(s)!= 11234.)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = conv("-.11234",buf);
  if ((int)(algo::to_float(s)/-.11234+.1)!=1)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  const istring cs = conv("QWErtyUIOP",buf);
  if (algo::to_lower(cs).compare(conv("qwertyuiop",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  if (algo::to_upper(cs).compare(conv("QWERTYUIOP",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  // 10
  t++;
  s = cs;
  algo::to_lower(s);
  if (s.compare(conv("qwertyuiop",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  algo::to_upper(s);
  if (s.compare(conv("QWERTYUIOP",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  if (!algo::equal_ignore_case(cs,s))
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s += ' ';
  if (!algo::equal_ignore_case_space(cs,s))
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s.insert(3,conv("  ",buf));
  if (!algo::equal_ignore_case_space(cs,s))
    cout<<"["<<t<<"] ERROR\n";

  // 15
  t++;
  s = conv("Tyu",buf);
  if (algo::find(cs,s) != 4)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s= conv("tyU",buf);
  if (algo::find(cs,s, 0, algo::SM_SENSITIVE) != 4)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s= conv("UIo",buf);
  if (algo::rfind(cs,s) != 8)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s= conv("UIO",buf);
  if (algo::rfind(cs,s, -1, algo::SM_SENSITIVE) != 8)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s= conv("ssfdgfdsfdsdfds",buf);
   istring ss = conv("Fd",buf);
  vector<size_t> v;
  algo::multi_find(s, ss, v);
  if (v.size()!=4)
    cout<<"["<<t<<"] ERROR A\n";
  if (v[2]!=8)
    cout<<"["<<t<<"] ERROR B\n";

  // 20
  t++;  
  algo::multi_find(s, ss, v, 3);
  if (v.size()!=3)
    cout<<"["<<t<<"] ERROR A\n";
  if (v[2]!=12)
    cout<<"["<<t<<"] ERROR B\n";

  t++;
  ss = conv("fd",buf);
  algo::multi_find(s, ss, v, 3, algo::SM_SENSITIVE);
  if (v.size()!=3)
    cout<<"["<<t<<"] ERROR A\n";
  if (v[2]!=12)
    cout<<"["<<t<<"] ERROR B\n";

  t++;
  ss = conv("fD",buf);
  if (!algo::start_with(s, ss, 2))
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("fD",buf);
  if (!algo::end_with(s, ss, 13))
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  if (algo::num_occurrence(s, ss) != 4)
    cout<<"["<<t<<"] ERROR\n";

  // 25
  t++;
  ss = conv("fd",buf);
  if (algo::num_occurrence(s, ss, 3, algo::SM_SENSITIVE) != 3)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
   vector<istring> iv(3);
  iv[0] = conv("fS",buf);
  iv[1] = conv("sG",buf);
  iv[2] = conv("rT",buf);
  algo::frequency_counter(conv("fsgsgdfsgrtsdfsgdfsg",buf), iv, v);
  if (v[0] != 4 || v[1]!=5 || v[2]!=1)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = conv("kdjdkdjksdkfjskddfu",buf);
  algo::substitute_char('k', '*', ss);
  if (ss.compare(conv("*djd*dj*sd*fjs*ddfu",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("kdjdkdjksdkfjsKddfu",buf);
  algo::substitute_char('k', '*', ss, 1, algo::SM_SENSITIVE);
  if (ss.compare(conv("kdjd*dj*sd*fjsKddfu",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = conv("aabcdsfklsabcklsdlabckldabcsldkabc",buf);
  algo::multi_substitute_string(conv("abc",buf),conv("*",buf), ss);
  if (ss.compare(conv("a*dsfkls*klsdl*kld*sldk*",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  // 30
  t++;
  ss = conv("aabcdsfklsAbcklsdlabckldaBcsldkabc",buf);
  algo::multi_substitute_string(conv("abc",buf),conv("****",buf), ss, 2, algo::SM_SENSITIVE);
  if (ss.compare(conv("aabcdsfklsAbcklsdl****kldaBcsldk****",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = conv("aabcdsfklsAbcklsdlabckldaBcsldkabc",buf);
  algo::substitute_string(conv("abc",buf),conv("****",buf), ss, 2, algo::SM_SENSITIVE);
  if (ss.compare(conv("aabcdsfklsAbcklsdl****kldaBcsldkabc",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = conv("aAbcdsfklsAbcklsdlabckldaBcsldkabc",buf);
  algo::substitute_string(conv("abc",buf),conv("****",buf), ss);
  if (ss.compare(conv("a****dsfklsAbcklsdlabckldaBcsldkabc",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = conv("aAbcdsfklsAbckl\ndlabck\nldaBcsldkabc\n",buf);
  algo::remove_newlines(ss);
  if (ss.compare(conv("aAbcdsfklsAbckl dlabck ldaBcsldkabc ",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = conv("qwergggtgygguiopgggg",buf);
  algo::remove_duplicate_chars(ss, 'g');
  if (ss.compare(conv("qwergtgyguiopg",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  // 35
  t++;
  ss = conv("  qwergg g   t  gyggui  opgggg    ",buf);
  algo::remove_multi_whitespace(ss);
  if (ss.compare(conv(" qwergg g t gyggui opgggg ",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("  qwergg g   t  gyggui    ",buf);
  algo::make_tokens_with_delimiter(ss, conv(" ",buf), iv);
  if (iv.size()!=4 || iv[0].compare(conv("qwergg",buf))!=0
      || iv[1].compare(conv("g",buf))!=0 || iv[2].compare(conv("t",buf))!=0
      ||iv[3].compare(conv("gyggui",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("sdfohdfghsdo",buf);
  algo::cut_range(ss, 2, 3);
  if (ss.compare(conv("fo",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("sdfohdfghsdo",buf);
  algo::cut_range(ss, 3, 3);
  if (ss.compare(conv("o",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("sdfohdfghsdo",buf);
  algo::cut_delimiter(ss, 3, 'f');
  if (ss.compare(conv("ohd",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  // 40
  t++;
  ss = conv("sdfohdifghsdio",buf);
  algo::cut_between_words(ss, conv("fo",buf), conv("di",buf));
  if (ss.compare(conv("hdifghs",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("sdfohdifghsdio",buf);
  algo::cut_between_words(ss, conv("hs",buf), conv("di",buf));
  if (ss.compare(conv("",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("sdfohdi\nfghs\ndio",buf);
  algo::cut_line_after(ss, conv("\n",buf));
  if (ss.compare(conv("fghs",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("sdf ohdif\nghs dio\n",buf);
  algo::take_between_marks(ss, ' ', '\n', iv);
  if (iv.size()!=2 || iv[0].compare(conv("ohdif",buf))!=0 || iv[1].compare(conv("dio",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("sdf\nohdif\nghsdio\n\n",buf);
  algo::take_between_mark(ss,'\n', iv);
  if (iv.size()!=3 || iv[0].compare(conv("ohdif",buf))!=0 || iv[1].compare(conv("ghsdio",buf))!=0
      || iv[2].compare(conv("",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  // 45
  t++;
  ss = conv("sdf\nohdif\nghsdio\n\n",buf);
  algo::reverse_cut_rear_with(ss,'s');
  if (ss.compare(conv("dio\n\n",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = conv("sdf\nohdif\nghsdio\n\n",buf);
  algo::reverse_cut_front_with(ss,'s');
  if (ss.compare(conv("sdf\nohdif\ngh",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("   sdfdfgdfgdfg    ",buf);
  algo::compact_head(ss);
  if (ss.compare(conv("sdfdfgdfgdfg    ",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("   sdfdfgdfgdfg    ",buf);
  algo::compact_tail(ss);
  if (ss.compare(conv("   sdfdfgdfgdfg",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = conv("   sdfdf  gdfgdfg    ",buf);
  algo::compact(ss);
  if (ss.compare(conv("sdfdf  gdfgdfg",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  // 50
  t++;
  ss = conv("   sdfdf  gdfgdfg    ", buf);
  algo::trim(ss);
  if (ss.compare(conv("sdfdfgdfgdfg",buf))!=0)
    cout<<"["<<t<<"] ERROR\n";

  //t++;
  //algo::write_to_file("hh", iv);
  
  t++;
  std::ifstream file ("./big5.txt", std::ios::in|std::ios::binary|std::ios::ate);
  file.seekg (0, std::ios::end);
  std::size_t length = file.tellg();
  file.seekg (0, std::ios::beg);
  // allocate memory:
  
  char* buffer = (char*)hlmalloc(length);
  // read data as a block:
  file.read (buffer,length);
  file.close();
  
  if (!algo::read_from_encode("big5", buffer,length, ss))
    cout<<"["<<t<<"] ERROR\n";
  hlfree(buffer);
  //algo::display(ss, "utf-8");
  
  
  char* buf = NULL;
  size_t len = 0;
  algo::write_to_encode("utf-8", &buf, len, ss);
  std::ofstream of ("utf-8.txt");
  of.write(buf, len);
  of.close();
  algo::release(buf);
  
  cout<<"izene_istring_algorithm_check is done!\n";
}

const size_t size = 1000;
const size_t bb = 50;
const size_t scale = 100000;
typedef vector_string<CharT, 1, bb> vectorString;
//typedef std::string vectorString;
const char* title = "vector_string<CharT, 1, ";
//const char* title = "std::string ";//"vector_string<CharT, 1, ";


BOOST_AUTO_TEST_CASE(izene_istring_iterator_perfomance_check)
{
  CharT* ch = new CharT[size*100];
  for (size_t i=0; i<size*100; i++)
    ch[i] = 'a'+rand()%26;

  vectorString str(ch, size*100);

  clock_t start, finish;
  
  
  start = clock();
  for (size_t i=0; i<scale; i++)
    for (vectorString::const_iterator j=str.begin(); j!=str.end(); j++)
      const CharT c = *j;
  finish = clock();

  cout<<title<<bb<<">::iterator["<<size*100<<"*"<<scale<<"]: "<<(double)(finish - start) / CLOCKS_PER_SEC<<endl;
  
  start = clock();
  for (size_t i=0; i<scale; i++)
    for (size_t j=0; j<size*100; j++)
      const CharT c = str[j];
  finish = clock();

  cout<<title<<bb<<">::operator[] ["<<size*100<<"*"<<scale<<"]: "<<(double)(finish - start) / CLOCKS_PER_SEC<<endl;
  delete ch;
}


BOOST_AUTO_TEST_CASE(izene_istring_append_perfomance_check)
{
  CharT* ch = new CharT[size];
  for (size_t i=0; i<size; i++)
    ch[i] = 'a'+rand()%26;
  
  vectorString str(ch, size);
  vectorString s(ch, size);

  clock_t start, finish;
  start = clock();
  for (size_t i=0; i<scale; i++)
    str += s;
  
  finish = clock();

  cout<<title<<bb<<">::append["<<size<<"*"<<scale<<"]: "<<(double)(finish - start) / CLOCKS_PER_SEC<<" "<<str.length()<<" "<<str.capacity()<<endl;
  delete ch;
}

BOOST_AUTO_TEST_CASE(izene_istring_substr_perfomance_check)
{  
  CharT* ch = new CharT[scale*100];
  for (size_t i=0; i<scale*100; i++)
    ch[i] = 'a'+rand()%26;
  
  vectorString str(ch, scale*100);

  clock_t start, finish;
  start = clock();
  for (size_t i=0; i<scale; i++)
    str.substr(i, i+100);
  finish = clock();

  cout<<title<<bb<<">::substr["<<scale*100<<"*"<<scale<<"]: "<<(double)(finish - start) / CLOCKS_PER_SEC<<endl;
  delete ch;
}

BOOST_AUTO_TEST_CASE(izene_istring_insert_perfomance_check)
{
  CharT* ch = new CharT[size];
  for (size_t i=0; i<size; i++)
    ch[i] = 'a'+rand()%26;

  vectorString s(ch, size);
  vectorString str(ch, size);

  clock_t start, finish;
  start = clock();
  for (size_t i=0; i<scale; i++)
  {
    str.insert(i, s);
  }
  finish = clock();

  cout<<title<<bb<<">::insert["<<size<<"*"<<scale<<"]: "<<(double)(finish - start) / CLOCKS_PER_SEC<<endl;
  delete ch;
}

// vector_string<CharT, 1, 50>::append[1000*100000]: 0.08
// vector_string<CharT, 1, 50>::iterator[100000*100000]: 0
// vector_string<CharT, 1, 50>::operator[] [100000*100000]: 13.53
// vector_string<CharT, 1, 50>::substr[10000000*100000]: 0

BOOST_AUTO_TEST_SUITE_END()

