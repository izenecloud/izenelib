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


using namespace std;
using namespace izenelib::am;

BOOST_AUTO_TEST_SUITE( sdb_trie_suite )

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
    for(size_t kkk=0; kkk<idListNum; kkk++) \
      BOOST_CHECK_EQUAL(result[kkk], idList[kkk]); \
  }\
}

BOOST_AUTO_TEST_CASE(SDBTrie_update)
{
    remove("sdbtrie_update*.sdb");

    {
      SDBTrie2<string,int> trie("./sdbtrie_update");
      trie.insert("apple",1);
    }

    {
      SDBTrie2<string,int> trie("./sdbtrie_update");
      BOOST_CHECK_EQUAL(trie.num_items(),  (size_t)1);
      TEST_TRIE_FIND("apple", 1);
    }

    {
      SDBTrie2<string,int> trie("./sdbtrie_update");
      trie.update("apple",2);
    }

    {
      SDBTrie2<string,int> trie("./sdbtrie_update");
      BOOST_CHECK_EQUAL(trie.num_items(),  (size_t)1);
      TEST_TRIE_FIND("apple", 2);
    }

    remove("sdbtrie_update*.sdb");
}



BOOST_AUTO_TEST_CASE(SDBTrie_find)
{
    remove("sdbtrie_insert*.sdb");

    {
      SDBTrie2<string,int> trie("./sdbtrie_insert");
      trie.insert("apple",1);
      trie.insert("blue",2);
      trie.insert("at",3);
      trie.insert("destination",4);
      trie.insert("earth",5);
      trie.insert("art",6);
      trie.insert("desk",7);
    }

    {
      SDBTrie2<string,int> trie("./sdbtrie_insert");
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

    remove("sdbtrie_insert*.sdb");
}


BOOST_AUTO_TEST_CASE(SDBTrie_prefixIterate)
{
    remove("sdbtrie_prefix*.sdb");

    {
      SDBTrie2<string,int> trie("./sdbtrie_prefix");
      trie.insert("apple",1);
      trie.insert("blue",2);
      trie.insert("at",3);
      trie.insert("destination",4);
      trie.insert("earth",5);
      trie.insert("art",6);
      trie.insert("desk",7);
    }

    {
      SDBTrie2<string,int> trie("./sdbtrie_prefix");
      BOOST_CHECK_EQUAL(trie.num_items(),  (size_t)7);

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

    remove("sdbtrie_prefix*.sdb");
}

BOOST_AUTO_TEST_SUITE_END()
