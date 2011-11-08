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

#include <am/tc/BTree.h>
#include <am/range/AmIterator.h>

#define DIR_PREFIX "./tmp/am_tc_Db_"

using namespace izenelib::am::tc;
using namespace izenelib::am;
using namespace std;
namespace bfs = boost::filesystem;

using namespace boost::unit_test;


BOOST_AUTO_TEST_SUITE(tc_Db_test)

BOOST_AUTO_TEST_CASE(BTreeIterator)
{
    bfs::path db_dir(DIR_PREFIX);
    boost::filesystem::remove_all(db_dir);
    bfs::create_directories(db_dir);
    std::string db_dir_str = db_dir.string();
    typedef BTree<std::string,std::string> BTreeType;
    BTreeType h(db_dir_str+"/BTreeIterator_test");
    BOOST_CHECK(h.open());

    {
        std::string key("1");
        std::string value("a");
        BOOST_CHECK(h.insert(key, value));
        BOOST_CHECK(h.size() == 1);
    }

    {
        std::string key("2");
        std::string value("b");
		
        BOOST_CHECK(h.insert(key, value));
        BOOST_CHECK(h.size() == 2);
    }

    {
        std::string key("3");
        std::string value("c");
        BOOST_CHECK(h.insert(key, value));
        BOOST_CHECK(h.size() == 3);
    }
    std::cout<<"Forward Iterator for TC from beginning"<<std::endl;		
    typedef AMForwardIterator<BTreeType > AMIteratorType;
    AMIteratorType iter(h);
    AMIteratorType end;
    for(; iter != end; ++iter)
    {
        const std::string& k = iter->first;
        const std::string& v = iter->second;
        std::cout<<k<<" "<<v<<std::endl;
    }

    std::cout<<"Forward Iterator for TC with start"<<std::endl;	
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
    std::cout<<"Reverse Iterator for TC"<<std::endl;	
    for(; iter3 != end2; ++iter3)
    {
        const std::string& k = iter3->first;
        const std::string& v = iter3->second;
        std::cout<<k<<" "<<v<<std::endl;
    }

}

BOOST_AUTO_TEST_SUITE_END() // tc_Db_test
