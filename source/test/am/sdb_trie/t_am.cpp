#include <string>

#include <boost/test/unit_test.hpp>

#include <am/sdb_trie/sdb_trie.hpp>
#include <am/map/map.hpp>
#include <am/btree/BTreeFile.h>
#include <am/sdb_btree/sdb_btree.h>
#include <am/sdb_hash/sdb_hash.h>

#include <am/am_test/am_test.h>


using namespace std;
using namespace izenelib::am;
using namespace izenelib::am_test;

typedef AmTest<string, uint32_t, SDBTrie2<string, uint32_t>, true> SDBTrieAmTest;
typedef AmTest<string, uint32_t, Map<string, uint32_t>, false> MapAmTest;

typedef AmTest<string, uint32_t, sdb_btree<string, uint32_t>, true> SdbBTreeAmTest;
typedef AmTest<string, uint32_t, sdb_hash<string, uint32_t>, true> SdbHashAmTest;

BOOST_AUTO_TEST_SUITE( sdb_trie_suite )

BOOST_AUTO_TEST_CASE(SDBTrie_am)
{
    std::cout << std::endl << "Performance Test for SDBTrie" << std::endl;
    SDBTrieAmTest amTest;
    amTest.setRandom(true);
    amTest.setNum(10000000);
    amTest.run_insert();
    amTest.run_find();
}

//BOOST_AUTO_TEST_CASE(Map_am)
//{
//    std::cout << std::endl << "Performance Test for Map" << std::endl;
//    MapAmTest amTest;
//    amTest.setRandom(true);
//    amTest.setNum(10000000);
//    amTest.run_insert();
//    amTest.run_find();
//}

//BOOST_AUTO_TEST_CASE(SdbBtree_am)
//{
//    std::cout << std::endl << "Performance Test for SdbBtree" << std::endl ;
//    SdbBTreeAmTest amTest;
//    amTest.setRandom(true);
//    amTest.setNum(10000000);
//    amTest.run_insert();
//    amTest.run_find();
//}

//BOOST_AUTO_TEST_CASE(SdbHash_am)
//{
//    std::cout << std::endl << "Performance Test for SDBHash" << std::endl;
//    SdbHashAmTest amTest;
//    amTest.setRandom(true);
//    amTest.setNum(10000000);
//    amTest.run_insert();
//    amTest.run_find();
//}
//

BOOST_AUTO_TEST_SUITE_END()
