#include <string>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <signal.h>

#include <boost/test/unit_test.hpp>

#include <am/trie/b_trie.hpp>

using namespace std;
using namespace izenelib::am;

BOOST_AUTO_TEST_SUITE( b_trie_suite )

#define testNumbers 10
static string testWords[testNumbers]= {"hello", "world", "this", "is", "a", "test", "for", "english", "b", "trie"};

#define TEST_TRIE_FIND(str, id) \
{ \
  uint64_t result = trie.find(str); \
  BOOST_CHECK_EQUAL(result, (uint64_t)(id) ); \
}

#define TEST_TRIE_FINDREGEXP(str, count) \
{ \
  vector<item_pair<string> > result; \
  trie.findRegExp(str, result); \
  BOOST_CHECK_EQUAL((unsigned int)result.size(), (unsigned int)(count) ); \
}

/**
 *This is for testing the correction of insertion and querying of Ustring version B-trie.
 *
 **/
BOOST_AUTO_TEST_CASE(BTrie_En_simple)
{
    remove("test_btrie_en.buk");
    remove("test_btrie_en.nod");
    remove("test_btrie_en.has");

    {
        BTrie_En trie("./test_btrie_en");

        int id = 1;
        for (int i=0; i<testNumbers; i++,id++)
            trie.insert(testWords[i],id);
        trie.flush();
    }

    {
        BTrie_En trie("./test_btrie_en");

        /** test BTrie_En::find() */
        TEST_TRIE_FIND("hello", 1);
        TEST_TRIE_FIND("trie", 10);
        TEST_TRIE_FIND("english", 8);
        TEST_TRIE_FIND("a", 5);
        TEST_TRIE_FIND("triee", -1);
        TEST_TRIE_FIND("woeld", -1);

        /** test BTrie_En::findRegExp() */
        //TEST_TRIE_FINDREGEXP("wo?ld", 1);  //fail ?
        //TEST_TRIE_FINDREGEXP("h?llo", 1); //fail ?
        TEST_TRIE_FINDREGEXP("wo*ld", 1);
        TEST_TRIE_FINDREGEXP("wo?d", 0);
        TEST_TRIE_FINDREGEXP("wo*d", 1);
        TEST_TRIE_FINDREGEXP("*l*", 3);
        TEST_TRIE_FINDREGEXP("t*", 3);
        TEST_TRIE_FINDREGEXP("*is", 2);
    }

    remove("test_btrie_en.buk");
    remove("test_btrie_en.nod");
    remove("test_btrie_en.has");
}

BOOST_AUTO_TEST_SUITE_END()
