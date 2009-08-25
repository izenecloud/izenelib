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

#include <am/sdb_trie/sdb_suffixtrie.h>


using namespace std;
using namespace izenelib::am;

BOOST_AUTO_TEST_SUITE( sdb_suffixtrie_suite )

#define CLEAN_SDB_FILE(test) \
{ \
    std::string testname = test;\
    remove( ("sdbsuffixtrie_" + testname + ".edge.table.sdb").c_str() ); \
    remove( ("sdbsuffixtrie_" + testname + ".isword.table.sdb").c_str() ); \
    remove( ("sdbsuffixtrie_" + testname + ".suffix2word.table.sdb").c_str() ); \
    remove( ("sdbsuffixtrie_" + testname + ".isword2.table.sdb").c_str() ); \
    remove( ("sdbsuffixtrie_" + testname + ".userdata.table.sdb").c_str() ); \
}


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

#define TEST_TRIE_PREFIX_ITERATE(str, idList, idListNum) \
{ \
  vector<int> result;\
  bool suc;\
  suc = trie.prefixIterate(str,result); \
  if(suc == false) \
    BOOST_CHECK_EQUAL(0, idListNum); \
  else\
  { \
    BOOST_CHECK_EQUAL(result.size(), (size_t)idListNum);\
    if(idListNum == result.size()) \
      for(size_t kkk=0; kkk<idListNum; kkk++) \
        BOOST_CHECK_EQUAL(result[kkk], idList[kkk]); \
    else { \
      std::cout << "should be ";\
      for(size_t kkk = 0; kkk<idListNum; kkk++) \
        std::cout << idList[kkk] << " "; \
      std::cout << std::endl << "but is "; \
      for(size_t kkk = 0; kkk<result.size(); kkk++) \
        std::cout << result[kkk] << " "; \
      std::cout << std::endl; \
    }\
  }\
}

#define TEST_TRIE_SUFFIX_ITERATE(str, idList, idListNum) \
{ \
  vector<int> result;\
  bool suc;\
  suc = trie.suffixIterate(str,result); \
  if(suc == false) \
    BOOST_CHECK_EQUAL(0, idListNum); \
  else\
  { \
    BOOST_CHECK_EQUAL(result.size(), (size_t)idListNum);\
    if(idListNum == result.size()) \
      for(size_t kkk=0; kkk<idListNum; kkk++) \
        BOOST_CHECK_EQUAL(result[kkk], idList[kkk]); \
    else { \
      std::cout << "should be ";\
      for(size_t kkk = 0; kkk<idListNum; kkk++) \
        std::cout << idList[kkk] << " "; \
      std::cout << std::endl << "but is "; \
      for(size_t kkk = 0; kkk<result.size(); kkk++) \
        std::cout << result[kkk] << " "; \
      std::cout << std::endl; \
    }\
  }\
}

#define TEST_TRIE_SUBSTRING_ITERATE(str, idList, idListNum) \
{ \
  vector<int> result;\
  bool suc;\
  suc = trie.substringIterate(str,result); \
  if(suc == false) \
    BOOST_CHECK_EQUAL(0, idListNum); \
  else\
  { \
    BOOST_CHECK_EQUAL(result.size(), (size_t)idListNum);\
    if(idListNum == result.size()) \
      for(size_t kkk=0; kkk<idListNum; kkk++) \
        BOOST_CHECK_EQUAL(result[kkk], idList[kkk]); \
    else { \
      std::cout << "should be ";\
      for(size_t kkk = 0; kkk<idListNum; kkk++) \
        std::cout << idList[kkk] << " "; \
      std::cout << std::endl << "but is "; \
      for(size_t kkk = 0; kkk<result.size(); kkk++) \
        std::cout << result[kkk] << " "; \
      std::cout << std::endl; \
    }\
  }\
}

BOOST_AUTO_TEST_CASE(SDBSuffixTrie_update)
{
    CLEAN_SDB_FILE("update");

    {
      SDBSuffixTrie2<string,int> trie("./sdbsuffixtrie_update");
      trie.insert("apple",1);
    }

    {
      SDBSuffixTrie2<string,int> trie("./sdbsuffixtrie_update");
      BOOST_CHECK_EQUAL(trie.num_items(),  (size_t)1);
      TEST_TRIE_FIND("apple", 1);
    }

    {
      SDBSuffixTrie2<string,int> trie("./sdbsuffixtrie_update");
      trie.update("apple",2);
    }

    {
      SDBSuffixTrie2<string,int> trie("./sdbsuffixtrie_update");
      BOOST_CHECK_EQUAL(trie.num_items(),  (size_t)1);
      TEST_TRIE_FIND("apple", 2);
    }

    CLEAN_SDB_FILE("update");
}



BOOST_AUTO_TEST_CASE(SDBSuffixTrie_find)
{
    CLEAN_SDB_FILE("find");

    {
      SDBSuffixTrie2<string,int> trie("./sdbsuffixtrie_find");
      trie.insert("apple",1);
      trie.insert("blue",2);
      trie.insert("at",3);
      trie.insert("destination",4);
      trie.insert("earth",5);
      trie.insert("art",6);
      trie.insert("desk",7);
    }

    {
      SDBSuffixTrie2<string,int> trie("./sdbsuffixtrie_find");
      BOOST_CHECK_EQUAL(trie.num_items(),  (size_t)7);

      TEST_TRIE_FIND("apple", 1);
      TEST_TRIE_FIND("blue", 2);
      TEST_TRIE_FIND("at", 3);
      TEST_TRIE_FIND("destination", 4);
      TEST_TRIE_FIND("earth", 5);
      TEST_TRIE_FIND("art", 6);
      TEST_TRIE_FIND("desk", 7);

      TEST_TRIE_FIND("appl", -1);
      TEST_TRIE_FIND("ap", -1);
      TEST_TRIE_FIND("a", -1);
      TEST_TRIE_FIND("d", -1);
      TEST_TRIE_FIND("des", -1);
      TEST_TRIE_FIND("c", -1);
      TEST_TRIE_FIND("bluee", -1);
    }

    CLEAN_SDB_FILE("find");
}


BOOST_AUTO_TEST_CASE(SDBTrie_prefixIterate)
{
    CLEAN_SDB_FILE("prefix");

    {
      SDBSuffixTrie2<string,int> trie("./sdbsuffixtrie_prefix");
      trie.insert("apple",1);
      trie.insert("blue",2);
      trie.insert("at",3);
      trie.insert("destination",4);
      trie.insert("earth",5);
      trie.insert("art",6);
      trie.insert("desk",7);
    }

    {
      SDBSuffixTrie2<string,int> trie("./sdbsuffixtrie_prefix");

      int idList1[3] = {1,6,3};
      TEST_TRIE_PREFIX_ITERATE("a", idList1, 3);
      int idList2[7] = {1,6,3,2,7,4,5};
      TEST_TRIE_PREFIX_ITERATE("", idList2, 7);
      int idList3[2] = {7,4};
      TEST_TRIE_PREFIX_ITERATE("des", idList3, 2);
      int idList4[1] = {2};
      TEST_TRIE_PREFIX_ITERATE("blue", idList4, 1);
      int idList5[0] = {};
      TEST_TRIE_PREFIX_ITERATE("eartha", idList5, 0);
      TEST_TRIE_PREFIX_ITERATE("appe", idList5, 0);
      TEST_TRIE_PREFIX_ITERATE("f", idList5, 0);
      TEST_TRIE_PREFIX_ITERATE("esk", idList5, 0);
    }

    CLEAN_SDB_FILE("prefix");
}


BOOST_AUTO_TEST_CASE(SDBTrie_suffixIterate)
{
    CLEAN_SDB_FILE("suffix");

    {
      SDBSuffixTrie2<string,int> trie("./sdbsuffixtrie_suffix");
      trie.insert("abcde",1);
      trie.insert("bcde",2);
      trie.insert("bcd",3);
      trie.insert("xabcde",4);
      trie.insert("ybcde",5);
    }

    {
      SDBSuffixTrie2<string,int> trie("./sdbsuffixtrie_suffix");

      int idList1[1] = {3};
      TEST_TRIE_SUFFIX_ITERATE("cd", idList1, 1);
      int idList2[4] = {1,2,4,5};
      TEST_TRIE_SUFFIX_ITERATE("e", idList2, 4);
      TEST_TRIE_SUFFIX_ITERATE("de", idList2, 4);
      TEST_TRIE_SUFFIX_ITERATE("cde", idList2, 4);
      TEST_TRIE_SUFFIX_ITERATE("bcde", idList2, 4);
      int idList3[2] = {1,4};
      TEST_TRIE_SUFFIX_ITERATE("abcde", idList3, 2);
      int idList4[1] = {3};
      TEST_TRIE_SUFFIX_ITERATE("bcd", idList4, 1);
      int idList5[0] = {};
      TEST_TRIE_SUFFIX_ITERATE("b", idList5, 0);
      TEST_TRIE_SUFFIX_ITERATE("xabc", idList5, 0);
      TEST_TRIE_SUFFIX_ITERATE("bc", idList5, 0);
      int idList6[5] = {1,3,2,4,5};
      TEST_TRIE_SUFFIX_ITERATE("", idList6, 5);
      int idList7[1] = {4};
      TEST_TRIE_SUFFIX_ITERATE("xabcde", idList7, 1);
    }

    CLEAN_SDB_FILE("suffix");

}



BOOST_AUTO_TEST_CASE(SDBTrie_substringIterate)
{
    CLEAN_SDB_FILE("substring");

    {
      SDBSuffixTrie2<string,int> trie("./sdbsuffixtrie_substring");
      trie.insert("abcde",1);
      trie.insert("bcde",2);
      trie.insert("bcd",3);
      trie.insert("xabcde",4);
      trie.insert("ybcde",5);
    }

    {
      SDBSuffixTrie2<string,int> trie("./sdbsuffixtrie_substring");

      int idList1[5] = {3,1,2,4,5};
      TEST_TRIE_SUBSTRING_ITERATE("cd", idList1, 5);
      TEST_TRIE_SUBSTRING_ITERATE("bc", idList1, 5);
      TEST_TRIE_SUBSTRING_ITERATE("bcd", idList1, 5);
      TEST_TRIE_SUBSTRING_ITERATE("b", idList1, 5);
      int idList2[4] = {1,2,4,5};
      TEST_TRIE_SUBSTRING_ITERATE("e", idList2, 4);
      TEST_TRIE_SUBSTRING_ITERATE("de", idList2, 4);
      TEST_TRIE_SUBSTRING_ITERATE("cde", idList2, 4);
      TEST_TRIE_SUBSTRING_ITERATE("bcde", idList2, 4);
      int idList3[2] = {1,4};
      TEST_TRIE_SUBSTRING_ITERATE("abcde", idList3, 2);
      int idList6[5] = {1,4,3,2,5};
      TEST_TRIE_SUBSTRING_ITERATE("", idList6, 5);
      int idList7[1] = {4};
      TEST_TRIE_SUBSTRING_ITERATE("xabcde", idList7, 1);
      TEST_TRIE_SUBSTRING_ITERATE("xabc", idList7, 1);
    }

    CLEAN_SDB_FILE("substring");

}

BOOST_AUTO_TEST_SUITE_END()
