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

#include <iostream>
#include <am/istring/vector_string.hpp>
#include <am/istring/deque_string.hpp>
#include <vector>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <fstream>

using namespace std;
using namespace izenelib::am;

BOOST_AUTO_TEST_SUITE( t_dstring_suite )

#define SCALE 10000000
#define MAX_LEN 1024

typedef char CharT;
const int BUCKET_BYTES = 64;
const int BUCKET_LENGTH = 64/sizeof(CharT);
typedef deque_string<CharT, 1, BUCKET_BYTES> istring;

BOOST_AUTO_TEST_CASE(izene_istring_construction_check)
{
  int t = 0;
  istring s = "asasasas";
  istring s1= s;

  if (s1.compare(s)!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  istring s2(s, 2, 3);
  if (s2.compare("asa")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  istring s3(s, 2);
  if (s3.compare("asasas")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  istring s4("sdfsfsfsdf", 5);
  if (s4.compare("sdfsf")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  istring s5(5, 'd');
  if (s5.compare("ddddd")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  string st("asasas");
  istring s6(st);
  if (s6.compare("asasas")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  vector<char> vt;
  vt.push_back('g');
  vt.push_back('f');
  vt.push_back('v');
  vt.push_back('d');
  vt.push_back('s');
  vt.push_back('w');
  
  istring s7(vt);
  if (s7.compare("gfvdsw")!=0)
    cout<<"["<<t<<"] ERROR\n";

  cout<<"izene_istring_construction_check is done!\n";
}

BOOST_AUTO_TEST_CASE(izene_istring_capacity_check)
{
  int t=0;
  istring is("dkfgjhghdkslsdjk");
  istring s = is;

  if (s.length()!=16)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  if (s.size() != 16)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s.resize(32);
  if (s.length()!=32
      || s.max_size()!=(32/BUCKET_LENGTH + (32%BUCKET_LENGTH==0? 0: 1))*BUCKET_LENGTH)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = is;
  is.resize(10);
  if (is.max_size()!= (10/BUCKET_LENGTH + (10%BUCKET_LENGTH==0? 0: 1))*BUCKET_LENGTH)
    cout<<"["<<t<<"] ERROR\n";

  
  t++;
  s = is;
  is.reserve(64);
  if (is.capacity()!=(64/BUCKET_LENGTH + (64%BUCKET_LENGTH==0? 0: 1))*BUCKET_LENGTH)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = is;
  s.resize(32, 'T');
  if (s.length()!=32
      || s.max_size()!= (32/BUCKET_LENGTH + (32%BUCKET_LENGTH==0? 0: 1))*BUCKET_LENGTH)
    cout<<"["<<t<<"] ERROR\n";

  cout<<"izene_istring_capacity_check is done!\n";
}

BOOST_AUTO_TEST_CASE(izene_istring_element_access_check)
{
  istring s("sfdfsfssfssfs");
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
  istring s("sfdfsfssfssfs");
  istring ss = s;
  int t= 0;

  ss += s;
  if (ss.compare("sfdfsfssfssfssfdfsfssfssfs")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = s;
  ss += "TTTT";
  if (ss.compare("sfdfsfssfssfsTTTT")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = s;
  ss += 'T';
  if (ss.compare("sfdfsfssfssfsT")!=0)
    cout<<"["<<t<<"] ERROR\n";
    
  t++;
  s += "kk";
  ss = s;
  ss.append(ss, 2, 3);
  if (ss.compare("sfdfsfssfssfskkdfs")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = ss;
  s.append("oweirer", 3);
  if (s.compare("sfdfsfssfssfskkdfsowe")!=0)
    cout<<"["<<t<<"] ERROR\n";

  // 5 
  t++;
  s = ss;
  s.append(4,' ');
  if (s.compare("sfdfsfssfssfskkdfs    ")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = ss;
  s.append(ss, 6);
  if (s.compare("sfdfsfssfssfskkdfsssfssfskkdfs")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = ss;
  s.push_back('o');
  s.push_back('A'); 
  if (s.compare("sfdfsfssfssfskkdfsoA")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = "fdfuiehjka";
  ss = s;
  ss.insert(3, s);
  if (ss.compare("fdfufdfuiehjkaiehjka")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = "fdfuiehjka";
  ss = s;
  s.insert(3, s);
  if (s.compare("fdfufdfuiehjkaiehjka")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  //------10
  t++;
  s = "fdfuiehjka";
  ss = s;
  ss.insert(3, "dsfsfs");
  if (ss.compare("fdfudsfsfsiehjka")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = "fdfuiehjka";
  ss = s;
  ss.insert(3, "dsfsfs", 3);
  if (ss.compare("fdfudsfiehjka")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = "fdfuiehjka";
  ss = s;
  ss.erase(3);
  if (ss.compare("fdf")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = "fdfuiehjka";
  ss = s;
  ss.erase(3, 3);
  if (ss.compare("fdfhjka")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = "fdfuiehjka";
  s.erase(3);
  if (s.compare("fdf")!=0)
    cout<<"["<<t<<"] ERROR\n";

  // 15
  t++;
  s = "fdfuiehjka";
  s.erase(3, 3);
  if (s.compare("fdfhjka")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = "fdfuiehjka";
  ss = "jj";
  s.replace(3, 5, ss);
  if (s.compare("fdfjjka")!=0)
    cout<<"["<<t<<"] ERROR\n";
  cout<<ss<<endl;

  t++;
  s = "fdfuiehjka";
  istring sss = s;
  ss = "jjj";
  sss.replace(3, 3, ss);
  if (sss.compare("fdfjjjhjka")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = "fdfuiehjka";
  ss = s;
  char j[3];
  ss.copy(j, 3, 3);
  istring ssss(j, 3);
  if (ssss.compare("uie")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s = "fdfuiehjka";
  ss = "sss";
  ss.swap(s);
  if (ss.compare("fdfuiehjka")!=0 || s.compare("sss")!=0)
    cout<<"["<<t<<"] ERROR\n";

  cout<<"izene_istring_modifiers_check is done!\n";
}

BOOST_AUTO_TEST_CASE(izene_istring_str_operation_check)
{
  istring s = "sdjkfjkdsjkfjkshkla";
  istring ss = "jkfj";
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
 
  istring s("ssdkfsdjghnkx");
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
  vector<char> vt;
  vt.push_back('g');
  vt.push_back('f');
  vt.push_back('v');
  vt.push_back('d');
  vt.push_back('s');
  vt.push_back('w');

  istring s;
  s.assign<vector<char>::iterator >(vt.begin(),vt.end());

  int t=0;
  if (s.compare("gfvdsw") != 0)
    cout<<"["<<t<<"] ERROR\n";

  s.insert(s.begin(), vt.begin(),vt.end());
  t++;
  if (s.compare("ggfvdswfvdsw") != 0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = "dkghk";
  istring ss;
  for (istring::reverse_iterator i=s.rbegin(); i<s.rend(); i++)
  {
    ss += *i;
  }
  if (ss.compare("khgkd") != 0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "";
  for (istring::const_iterator i=s.begin(); i<s.end(); i++)
  {
    ss += *i;
  }
  if (ss.compare("dkghk") != 0)
    cout<<"["<<t<<"] ERROR\n";
  
  cout<<"izene_istring_iterator_check is done!\n";
}

BOOST_AUTO_TEST_CASE(izene_istring_algorithms_check)
{

  typedef Algorithm<istring> algo;
  
  int t=0;
  
  istring s("-11013");
  if (algo::to_integer(s)!= -11013)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = "11234";
  if (algo::to_integer(s)!= 11234)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = "-11013";
  if (algo::to_long(s)!= -11013)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = "11234";
  if (algo::to_long(s)!= 11234)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = "11234";
  if (algo::to_ulong(s)!= 11234)
    cout<<"["<<t<<"] ERROR\n";

  // 5  
  t++;
  s = "1.1234";
  if ((int)(algo::to_float(s)/ 1.1234 +.1) !=1)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = "11234.";
  if (algo::to_float(s)!= 11234.)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s = "-.11234";
  if ((int)(algo::to_float(s)/-.11234+.1)!=1)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  const istring cs = "QWErtyUIOP";
  if (algo::to_lower(cs).compare("qwertyuiop")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  if (algo::to_upper(cs).compare("QWERTYUIOP")!=0)
    cout<<"["<<t<<"] ERROR\n";

  // 10
  t++;
  s = cs;
  algo::to_lower(s);
  if (s.compare("qwertyuiop")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  algo::to_upper(s);
  if (s.compare("QWERTYUIOP")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  if (!algo::equal_ignore_case(cs,s))
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s += ' ';
  if (!algo::equal_ignore_case_space(cs,s))
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s.insert(3,"  ");
  if (!algo::equal_ignore_case_space(cs,s))
    cout<<"["<<t<<"] ERROR\n";

  // 15
  t++;
  s = "Tyu";
  if (algo::find(cs,s) != 4)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s= "tyU";
  if (algo::find(cs,s, 0, algo::SM_SENSITIVE) != 4)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  s= "UIo";
  if (algo::rfind(cs,s) != 8)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s= "UIO";
  if (algo::rfind(cs,s, -1, algo::SM_SENSITIVE) != 8)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  s= "ssfdgfdsfdsdfds";
   istring ss = "Fd";
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
  ss = "fd";
  algo::multi_find(s, ss, v, 3, algo::SM_SENSITIVE);
  if (v.size()!=3)
    cout<<"["<<t<<"] ERROR A\n";
  if (v[2]!=12)
    cout<<"["<<t<<"] ERROR B\n";

  t++;
  ss = "fD";
  if (!algo::start_with(s, ss, 2))
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "fD";
  if (!algo::end_with(s, ss, 13))
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  if (algo::num_occurrence(s, ss) != 4)
    cout<<"["<<t<<"] ERROR\n";

  // 25
  t++;
  ss = "fd";
  if (algo::num_occurrence(s, ss, 3, algo::SM_SENSITIVE) != 3)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
   vector<istring> iv(3);
  iv[0] = "fS";
  iv[1] = "sG";
  iv[2] = "rT";
  algo::frequency_counter("fsgsgdfsgrtsdfsgdfsg", iv, v);
  if (v[0] != 4 || v[1]!=5 || v[2]!=1)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = "kdjdkdjksdkfjskddfu";
  algo::substitute_char('k', '*', ss);
  if (ss.compare("*djd*dj*sd*fjs*ddfu")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "kdjdkdjksdkfjsKddfu";
  algo::substitute_char('k', '*', ss, 1, algo::SM_SENSITIVE);
  if (ss.compare("kdjd*dj*sd*fjsKddfu")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = "aabcdsfklsabcklsdlabckldabcsldkabc";
  algo::multi_substitute_string("abc","*", ss);
  if (ss.compare("a*dsfkls*klsdl*kld*sldk*")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  // 30
  t++;
  ss = "aabcdsfklsAbcklsdlabckldaBcsldkabc";
  algo::multi_substitute_string("abc","****", ss, 2, algo::SM_SENSITIVE);
  if (ss.compare("aabcdsfklsAbcklsdl****kldaBcsldk****")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = "aabcdsfklsAbcklsdlabckldaBcsldkabc";
  algo::substitute_string("abc","****", ss, 2, algo::SM_SENSITIVE);
  if (ss.compare("aabcdsfklsAbcklsdl****kldaBcsldkabc")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = "aAbcdsfklsAbcklsdlabckldaBcsldkabc";
  algo::substitute_string("abc","****", ss);
  if (ss.compare("a****dsfklsAbcklsdlabckldaBcsldkabc")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = "aAbcdsfklsAbckl\ndlabck\nldaBcsldkabc\n";
  algo::remove_newlines(ss);
  if (ss.compare("aAbcdsfklsAbckl dlabck ldaBcsldkabc ")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = "qwergggtgygguiopgggg";
  algo::remove_duplicate_chars(ss, 'g');
  if (ss.compare("qwergtgyguiopg")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  // 35
  t++;
  ss = "  qwergg g   t  gyggui  opgggg    ";
  algo::remove_multi_whitespace(ss);
  if (ss.compare(" qwergg g t gyggui opgggg ")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "  qwergg g   t  gyggui    ";
  algo::make_tokens_with_delimiter(ss, " ", iv);
  if (iv.size()!=4 || iv[0].compare("qwergg")!=0
      || iv[1].compare("g")!=0 || iv[2].compare("t")!=0
      ||iv[3].compare("gyggui")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "sdfohdfghsdo";
  algo::cut_range(ss, 2, 3);
  if (ss.compare("fo")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "sdfohdfghsdo";
  algo::cut_range(ss, 3, 3);
  if (ss.compare("o")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "sdfohdfghsdo";
  algo::cut_delimiter(ss, 3, 'f');
  if (ss.compare("ohd")!=0)
    cout<<"["<<t<<"] ERROR\n";

  // 40
  t++;
  ss = "sdfohdifghsdio";
  algo::cut_between_words(ss, "fo", "di");
  if (ss.compare("hdifghs")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "sdfohdifghsdio";
  algo::cut_between_words(ss, "hs", "di");
  if (ss.compare("")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "sdfohdi\nfghs\ndio";
  algo::cut_line_after(ss, "\n");
  if (ss.compare("fghs")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "sdf ohdif\nghs dio\n";
  algo::take_between_marks(ss, ' ', '\n', iv);
  if (iv.size()!=2 || iv[0].compare("ohdif")!=0 || iv[1].compare("dio")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "sdf\nohdif\nghsdio\n\n";
  algo::take_between_mark(ss,'\n', iv);
  if (iv.size()!=3 || iv[0].compare("ohdif")!=0 || iv[1].compare("ghsdio")!=0
      || iv[2].compare("")!=0)
    cout<<"["<<t<<"] ERROR\n";

  // 45
  t++;
  ss = "sdf\nohdif\nghsdio\n\n";
  algo::reverse_cut_rear_with(ss,'s');
  if (ss.compare("dio\n\n")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = "sdf\nohdif\nghsdio\n\n";
  algo::reverse_cut_front_with(ss,'s');
  if (ss.compare("sdf\nohdif\ngh")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "   sdfdfgdfgdfg    ";
  algo::compact_head(ss);
  if (ss.compare("sdfdfgdfgdfg    ")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "   sdfdfgdfgdfg    ";
  algo::compact_tail(ss);
  if (ss.compare("   sdfdfgdfgdfg")!=0)
    cout<<"["<<t<<"] ERROR\n";

  t++;
  ss = "   sdfdf  gdfgdfg    ";
  algo::compact(ss);
  if (ss.compare("sdfdf  gdfgdfg")!=0)
    cout<<"["<<t<<"] ERROR\n";
  
  t++;
  ss = "   sdfdf  gdfgdfg    ";
  algo::trim(ss);
  if (ss.compare("sdfdfgdfgdfg")!=0)
    cout<<"["<<t<<"] ERROR\n";





  float e = 6037*12.;
  float r = 1.0252;
  float g = 0;
  for (int i=0; i<10; i++)
    g = g*r+e;
  cout<<(g)<<endl;
  
  cout<<"izene_istring_algorithm_check is done!\n";
}


BOOST_AUTO_TEST_SUITE_END()

