#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <string>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>

#include <cstdio>

#include <am/luxio/BTree.h>
#include <am/range/AmIterator.h>
#include <util/ClockTimer.h>

#define DIR_PREFIX "./tmp/am_luxio_Db_"

using namespace izenelib::am::luxio;
using namespace izenelib::am;
using namespace std;
namespace bfs = boost::filesystem;

using namespace boost::unit_test;


BOOST_AUTO_TEST_SUITE(luxio_Db_test)

BOOST_AUTO_TEST_CASE(BTree_test)
{
    bfs::path db_dir(DIR_PREFIX);
    boost::filesystem::remove_all(db_dir);
    bfs::create_directories(db_dir);
    std::string db_dir_str = db_dir.string();
    typedef BTree<std::string,std::string> BTreeType;
    BTreeType h(db_dir_str+"/BTree_test");
    BOOST_CHECK(h.open());

    {
        char key[] = "1";
        char value[] = "a";
        BOOST_CHECK(h.insert(std::string(key), std::string(value)));
        BOOST_CHECK(h.size() == 1);
    }

    {
        char key[] = "2";
        char value[] = "b";
        BOOST_CHECK(h.insert(std::string(key), std::string(value)));
        BOOST_CHECK(h.size() == 2);
    }

    {
        char key[] = "3";
        char value[] = "c";
        BOOST_CHECK(h.insert(std::string(key), std::string(value)));
        BOOST_CHECK(h.size() == 3);
    }

    {
        char key[] = "2";
        char value[] = "bb";
        BOOST_CHECK(h.update(std::string(key), std::string(value)));
        BOOST_CHECK(h.size() == 3);
    }

    {
        std::string k("1");
        std::string v;
        BOOST_CHECK(h.get(k, v));
        BOOST_CHECK(v == "a");
    }
    {
        std::string k("2");
        std::string v;
        BOOST_CHECK(h.get(k, v));
        BOOST_CHECK(v == "bb");
    }
    {
        std::string k("3");
        std::string v;
        BOOST_CHECK(h.get(k, v));
        BOOST_CHECK(v == "c");
    }

    typedef AMIterator<BTreeType > AMIteratorType;
    AMIteratorType iter(h);
    AMIteratorType end;
    for(; iter != end; ++iter)
    {
        const std::string& k = iter->first;
        const std::string& v = iter->second;
        std::cout<<k<<" "<<v<<std::endl;
    }

    AMIteratorType iter2(h,std::string("2"));
    for(; iter2 != end; ++iter2)
    {
        const std::string& k = iter2->first;
        const std::string& v = iter2->second;
        std::cout<<k<<" "<<v<<std::endl;
    }

    typedef AMReverseIterator<BTreeType > AMRIteratorType;
    AMRIteratorType iter3(h,std::string("3"));
    AMRIteratorType end2;
    for(; iter3 != end2; ++iter3)
    {
        const std::string& k = iter3->first;
        const std::string& v = iter3->second;
        std::cout<<k<<" "<<v<<std::endl;
    }

    AMRIteratorType iter4(h);
    for(; iter4 != end2; ++iter4)
    {
        const std::string& k = iter4->first;
        const std::string& v = iter4->second;
        std::cout<<k<<" "<<v<<std::endl;
    }

    {
    boost::filesystem::remove_all(db_dir);
    bfs::create_directories(db_dir);
    std::string db_dir_str = db_dir.string();
    BTreeType h(db_dir_str+"/BTree_test");
    h.set_index_type(BTreeType::NON_CLUSTER_PADDING);
    BOOST_CHECK(h.open());
    BOOST_CHECK(h.insert(std::string("1"), std::string("a")));

    std::string k("1");
    std::string v;
    BOOST_CHECK(h.get(k, v));
    BOOST_CHECK(v == "a");
    }

    {
    boost::filesystem::remove_all(db_dir);
    bfs::create_directories(db_dir);
    std::string db_dir_str = db_dir.string();
    BTreeType h(db_dir_str+"/BTree_test");
    h.set_index_type(BTreeType::CLUSTER);
    BOOST_CHECK(h.open());
    BOOST_CHECK(h.insert(std::string("1"), std::string("a")));

    std::string k("1");
    std::string v;
    BOOST_CHECK(h.get(k, v));
    BOOST_CHECK(v == "a");
    }
}

BOOST_AUTO_TEST_CASE(Btree_bench)
{
    bfs::path db_dir(DIR_PREFIX);
    boost::filesystem::remove_all(db_dir);
    bfs::create_directories(db_dir);
    std::string db_dir_str = db_dir.string();
    typedef BTree<uint32_t,uint32_t> BTreeType;
    BTreeType h(db_dir_str+"/BTree_test");
	
    BOOST_CHECK(h.open());

    size_t max = 4000;
    for(uint32_t i = 1; i < max; ++i)
    {
        h.insert(i,i);
    }

    std::cout<<"Begin iterating "<<std::endl;

    izenelib::util::ClockTimer timer;
    typedef AMIterator<BTreeType > AMIteratorType;
    AMIteratorType iter(h);
    AMIteratorType end;
    for(; iter != end; ++iter)
    {
    }
    std::cout<<"Iterating elapsed "<<timer.elapsed()<<std::endl;
}

BOOST_AUTO_TEST_SUITE_END() // luxio_Db_test
