#include <string>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <signal.h>

#include <boost/test/unit_test.hpp>

#include <am/trie/b_trie.hpp>

#include <util/ustring/UString.h>

using namespace std;
using namespace izenelib::util;
using namespace izenelib::am;

BOOST_AUTO_TEST_SUITE( b_trie_suite )

#define testNumbers 10
static string testWords[testNumbers]= {"你好", "世界", "本文件", "是", "一个", " 针对 ", "trie树", "的", "测试",  "文件"};

#define TEST_TRIE_FIND(str, id) \
{ \
  uint64_t result = trie.find(str); \
  BOOST_CHECK_EQUAL(result, (uint64_t)(id) ); \
}

#define TEST_TRIE_FINDREGEXP(str, count) \
{ \
  vector<item_pair<UString> > result; \
  trie.findRegExp(str, result); \
  BOOST_CHECK_EQUAL((unsigned int)result.size(), (unsigned int)(count) ); \
}

/**
 *This is for testing the correction of insertion and querying of Ustring version B-trie.
 *
 **/
BOOST_AUTO_TEST_CASE(BTrie_CH_simple)
{
    remove("test_btrie_ch.buk");
    remove("test_btrie_ch.nod");
    remove("test_btrie_ch.has");

    {
        BTrie_CJK trie("./test_btrie_ch");

        int id = 1;
        for (int i=0; i<testNumbers; i++,id++)
            trie.insert(UString(testWords[i], UString::UTF_8),id);
        trie.flush();
    }

    {
        BTrie_CJK trie("./test_btrie_ch");

        /** test BTrie_CH::find() */
        TEST_TRIE_FIND(UString("你好",UString::UTF_8), 1);
        TEST_TRIE_FIND(UString("文件",UString::UTF_8), 10);
        TEST_TRIE_FIND(UString("一个",UString::UTF_8), 5);
        TEST_TRIE_FIND(UString("你阿好",UString::UTF_8), -1);

        /** test BTrie_CH::findRegExp() */
        TEST_TRIE_FINDREGEXP(UString("*文件",UString::UTF_8), 2);
        TEST_TRIE_FINDREGEXP(UString("文*件",UString::UTF_8), 1);
        TEST_TRIE_FINDREGEXP(UString("文件*",UString::UTF_8), 1);
        TEST_TRIE_FINDREGEXP(UString("trie*",UString::UTF_8), 1);
    }

    remove("test_btrie_ch.buk");
    remove("test_btrie_ch.nod");
    remove("test_btrie_ch.has");
}

BOOST_AUTO_TEST_SUITE_END()
