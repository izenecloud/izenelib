#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <string>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>

#include <cstdio>

#include <am/lmdb/Mdb.h>
#include <am/range/AmIterator.h>

#define DIR_PREFIX "./tmp/am_lmdb_Db_"

using namespace izenelib::am::lmdb;
using namespace izenelib::am;
using namespace std;
namespace bfs = boost::filesystem;

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(lmdb_Db_test)

BOOST_AUTO_TEST_CASE(Simple_test)
{
    bfs::path db_dir(DIR_PREFIX);
    boost::filesystem::remove_all(db_dir);
    bfs::create_directories(db_dir);
    std::string db_dir_str = db_dir.string();

    typedef Mdb<int,int> MDBType;
    MDBType table(db_dir_str+"/lmdb");
    int size = 10000;
    int i;
    for (i = 1; i <= size; ++i) {
        BOOST_CHECK(table.insert(i,i*100));
    }
    cout<<"insert finished"<<endl;
    for (i = 1; i <= size; ++i) {
        int value;
        table.get(i, value);
        BOOST_CHECK_EQUAL(value, i*100);
    }

    cout<<"start iterating"<<endl;

    typedef AMIterator<MDBType > AMIteratorType;
    AMIteratorType iter(table);
    AMIteratorType end;
    int iterStep = 0;
    for(; iter != end; ++iter)
    {
	BOOST_CHECK_EQUAL(iter->second, iter->first*100);
	++iterStep;
    }
    BOOST_CHECK_EQUAL(iterStep, size);

    typedef AMReverseIterator<MDBType > AMRIteratorType;
    AMRIteratorType iter3(table);
    AMRIteratorType end2;
    iterStep = 0;
    for(; iter3 != end2; ++iter3)
    {
        BOOST_CHECK_EQUAL(iter->second, iter->first*100);
        ++iterStep;
    }
    BOOST_CHECK_EQUAL(iterStep, size);

}

BOOST_AUTO_TEST_SUITE_END() // lmdb_Db_test

