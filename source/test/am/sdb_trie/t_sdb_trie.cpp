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

#include <am/sdb_trie/sdb_trie.hpp>


using namespace std;
using namespace izenelib::am;

BOOST_AUTO_TEST_SUITE( sdb_trie_suite )

#define CLEAN_SDB_FILE(test) \
{ \
    std::string testname = test;\
    remove( ("sdbtrie_" + testname + ".sdb").c_str() ); \
}


#define TEST_TRIE_FIND(str, id) \
{ \
  int result;\
  bool suc;\
  suc = trie.get(str,result); \
  if(suc == false) \
    BOOST_CHECK_EQUAL(id,-1); \
  else\
    BOOST_CHECK_EQUAL(result, id); \
}

#define TEST_TRIE_SEARCH_REGEXP(str, idList, idListNum) \
{ \
  vector<int> result;\
  bool suc;\
  suc = trie.findRegExp(str,result); \
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
      if(idListNum == 0) std::cout << "null";\
      for(size_t kkk = 0; kkk<idListNum; kkk++) \
        std::cout << idList[kkk] << " "; \
      std::cout << std::endl << "but is "; \
      if(result.size() == 0) std::cout << "null";\
      for(size_t kkk = 0; kkk<result.size(); kkk++) \
        std::cout << result[kkk] << " "; \
      std::cout << std::endl; \
    }\
  }\
}

BOOST_AUTO_TEST_CASE(SDBTrie_update)
{
    CLEAN_SDB_FILE("update");

    {
      SDBTrie2<string,int> trie("./sdbtrie_update");
      trie.open();
      trie.insert("apple",1);
    }

    {
      SDBTrie2<string,int> trie("./sdbtrie_update");
      trie.open();
      BOOST_CHECK_EQUAL(trie.num_items(),  (size_t)1);
      TEST_TRIE_FIND("apple", 1);
    }

    {
      SDBTrie2<string,int> trie("./sdbtrie_update");
      trie.open();
      trie.update("apple",2);
    }

    {
      SDBTrie2<string,int> trie("./sdbtrie_update");
      trie.open();
      BOOST_CHECK_EQUAL(trie.num_items(),  (size_t)1);
      TEST_TRIE_FIND("apple", 2);
    }

    CLEAN_SDB_FILE("update");

}



BOOST_AUTO_TEST_CASE(SDBTrie_find)
{
    CLEAN_SDB_FILE("find");

    {
      SDBTrie2<string,int> trie("./sdbtrie_find");
      trie.open();
      trie.insert("apple",1);
      trie.insert("blue",2);
      trie.insert("at",3);
      trie.insert("destination",4);
      trie.insert("earth",5);
      trie.insert("art",6);
      trie.insert("desk",7);
    }

    {
      SDBTrie2<string,int> trie("./sdbtrie_find");
      trie.open();
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


BOOST_AUTO_TEST_CASE(SDBTrie_searchregexp)
{
    CLEAN_SDB_FILE("searchregexp");

    {
      SDBTrie2<string,int> trie("./sdbtrie_searchregexp");
      trie.open();
      trie.insert("apple",1);
      trie.insert("blue",2);
      trie.insert("at",3);
      trie.insert("destination",4);
      trie.insert("earth",5);
      trie.insert("art",6);
      trie.insert("desk",7);
    }

    {
      SDBTrie2<string,int> trie("./sdbtrie_searchregexp");
      trie.open();
      BOOST_CHECK_EQUAL(trie.num_items(),  (size_t)7);

      int idList1[3] = {1,6,3};
      TEST_TRIE_SEARCH_REGEXP("a*", idList1, 3);
      int idList2[7] = {1,6,3,2,7,4,5};
      TEST_TRIE_SEARCH_REGEXP("*", idList2, 7);
      int idList3[2] = {7,4};
      TEST_TRIE_SEARCH_REGEXP("des*", idList3, 2);
      int idList4[1] = {2};
      TEST_TRIE_SEARCH_REGEXP("blue*", idList4, 1);
      int idList5[0] = {};
      TEST_TRIE_SEARCH_REGEXP("eartha*", idList5, 0);
      TEST_TRIE_SEARCH_REGEXP("appe*", idList5, 0);
      TEST_TRIE_SEARCH_REGEXP("f*", idList5, 0);
      TEST_TRIE_SEARCH_REGEXP("esk*", idList5, 0);
      int idList6[5] = {1, 2, 7, 4, 5};
      TEST_TRIE_SEARCH_REGEXP("*e*", idList6, 5);
      int idList7[2] = {1, 2};
      TEST_TRIE_SEARCH_REGEXP("*e", idList7, 2);
      int idList8[1] = {5};
      TEST_TRIE_SEARCH_REGEXP("e*", idList8, 1);
      int idList10[4] = {4, 5, 6, 3};
      TEST_TRIE_SEARCH_REGEXP("*a*t*", idList10, 4);
      int idList11[2] = {4, 3};
      TEST_TRIE_SEARCH_REGEXP("*at*", idList11, 2);
      int idList12[1] = {5};
      TEST_TRIE_SEARCH_REGEXP("ea*th", idList12, 1);
      TEST_TRIE_SEARCH_REGEXP("ea?th", idList12, 1);
      int idList13[0] = {};
      TEST_TRIE_SEARCH_REGEXP("ear?th", idList13, 0);

    }

    CLEAN_SDB_FILE("searchregexp");
}

BOOST_AUTO_TEST_SUITE_END()
