/// @file   t_sdb_trie.cpp
/// @brief  A test unit for checking SDBTrie and SDBTrie2
/// @author Wei Cao
/// @date   2000-08-04
///
///
/// @details
///


#include <string>

#include <boost/test/unit_test.hpp>

#include <am/sdb_trie/sdb_trie.h>

#include <wiselib/ustring/UString.h>

using namespace std;
using namespace wiselib;
using namespace izenelib::am;

BOOST_AUTO_TEST_SUITE( sdb_trie_suite )

#define testNumbers 10
static string testWords[testNumbers]= {"hello", "world", "this", "is", "a", "test", "for", "english", "b", "trie"};

#define TEST_TRIE_FIND(str, id) \
{ \
  int result;\
  bool suc;\
  suc = trie.find(str,result); \
  if(suc == false) \
    BOOST_CHECK_EQUAL(id,-1); \
  else\
    BOOST_CHECK_EQUAL(result, id); \
}

//#define TEST_TRIE_FINDREGEXP(str, count) \
//{ \
//  vector<item_pair<UString> > result; \
//  trie.findRegExp(str, result); \
//  BOOST_CHECK_EQUAL((unsigned int)result.size(), (unsigned int)(count) ); \
//}

/**
 *This is for testing the correction of insertion and querying of Ustring version B-trie.
 *
 **/
BOOST_AUTO_TEST_CASE(SDBTrie_ustring)
{
    remove("test_sdbtrie_ustring*.sdb");

    {
      SDBTrie2<UString,int> trie("./sdbtrie_ustring");

      int id = 1;
      for (int i=0; i<testNumbers; i++,id++)
        trie.insert(UString(testWords[i],UString::UTF_8),id);
    }

    {
      SDBTrie2<UString,int> trie("./sdbtrie_ustring");

      /** test BTrie_En::find() */
      TEST_TRIE_FIND(UString("hello",UString::UTF_8), 1);
      TEST_TRIE_FIND(UString("trie",UString::UTF_8), 10);
      TEST_TRIE_FIND(UString("english",UString::UTF_8), 8);
      TEST_TRIE_FIND(UString("a",UString::UTF_8), 5);
      TEST_TRIE_FIND(UString("triee",UString::UTF_8), -1);
      TEST_TRIE_FIND(UString("woeld",UString::UTF_8), -1);

//      /** test BTrie_En::findRegExp() */
//      TEST_TRIE_FINDREGEXP(UString("wo?ld",UString::UTF_8), 1);
//      TEST_TRIE_FINDREGEXP(UString("h?llo",UString::UTF_8), 1);
//      TEST_TRIE_FINDREGEXP(UString("wo*ld",UString::UTF_8), 1);
//      TEST_TRIE_FINDREGEXP(UString("wo?d",UString::UTF_8), 0);
//      TEST_TRIE_FINDREGEXP(UString("wo*d",UString::UTF_8), 1);
//      TEST_TRIE_FINDREGEXP(UString("*l*",UString::UTF_8), 3);
//      TEST_TRIE_FINDREGEXP(UString("t*",UString::UTF_8), 3);
//      TEST_TRIE_FINDREGEXP(UString("*is",UString::UTF_8), 2);
    }

    remove("test_sdbtrie_ustring*.sdb");
}

BOOST_AUTO_TEST_SUITE_END()
