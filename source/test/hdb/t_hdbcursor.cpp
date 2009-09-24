/// @file   t_hdbcursor.cpp
/// @brief  A test unit for checking HDBCursor's correctness.
/// @author Wei Cao
/// @date   2000-09-24
///
///
/// @details
///

#include <string>

#include <boost/test/unit_test.hpp>

#include <wiselib/ustring/UString.h>

#include <hdb/HugeDB.h>


using namespace std;
using namespace wiselib;
using namespace izenelib::am;
using namespace izenelib::hdb;

BOOST_AUTO_TEST_SUITE( hdb_suite )

typedef ordered_hdb<int, int> HDBType;
typedef HDBType::HDBCursor HDBCursor;

BOOST_AUTO_TEST_CASE(hdb_seq_forward)
{

    HDBType hdb("t_hdb_seq_forward");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, 7*i);

    int count = 0;
    HDBCursor cursor = hdb.get_first_Locn();
    while(hdb.seq(cursor))
    {
        int k,v;
        hdb.get(cursor,k,v);
        BOOST_CHECK_EQUAL(k,count);
        BOOST_CHECK_EQUAL(v,7*count);
        count ++;
    }

    hdb.close();
}

BOOST_AUTO_TEST_CASE(hdb_search_forward)
{

    HDBType hdb("t_hdb_search_forward");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, 7*i);

    int k,v;

    {
        HDBCursor cursor = hdb.search(-1);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), true);
        BOOST_CHECK_EQUAL(k, 0);
        BOOST_CHECK_EQUAL(v, 0);
    }

    {
        HDBCursor cursor = hdb.search(0);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), true);
        BOOST_CHECK_EQUAL(k, 0);
        BOOST_CHECK_EQUAL(v, 0);
    }

    {
        HDBCursor cursor = hdb.search(100);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), true);
        BOOST_CHECK_EQUAL(k, 100);
        BOOST_CHECK_EQUAL(v, 700);
    }

    {
        HDBCursor cursor = hdb.search(800);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), true);
        BOOST_CHECK_EQUAL(k, 800);
        BOOST_CHECK_EQUAL(v, 5600);
    }


    {
        HDBCursor cursor = hdb.search(1000);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), false);
    }

    hdb.close();
}


BOOST_AUTO_TEST_CASE(hdb_getvaluebetween)
{

    HDBType hdb("t_hdb_getvaluebetween");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, 7*i);

    {
        std::vector<DataType<int,int> > result;
        hdb.getValueBetween(result,0,999);
        BOOST_CHECK_EQUAL(result.size(), 1000U);
        for(int i=0; i<1000; i++) {
            BOOST_CHECK_EQUAL(result[i].key,i);
            BOOST_CHECK_EQUAL(result[i].value,7*i);
        }
    }

    {
        std::vector<DataType<int,int> > result;
        hdb.getValueBetween(result,-1,1000);
        BOOST_CHECK_EQUAL(result.size(), 1000U);
        for(int i=0; i<1000; i++) {
            BOOST_CHECK_EQUAL(result[i].key,i);
            BOOST_CHECK_EQUAL(result[i].value,7*i);
        }
    }


    {
        std::vector<DataType<int,int> > result;
        hdb.getValueBetween(result,-1,998);
        BOOST_CHECK_EQUAL(result.size(), 999U);
        for(int i=0; i<999; i++) {
            BOOST_CHECK_EQUAL(result[i].key,i);
            BOOST_CHECK_EQUAL(result[i].value,7*i);
        }
    }

    {
        std::vector<DataType<int,int> > result;
        hdb.getValueBetween(result, 1,1000);
        BOOST_CHECK_EQUAL(result.size(), 999U);
        for(int i=0; i<999; i++) {
            BOOST_CHECK_EQUAL(result[i].key,i+1);
            BOOST_CHECK_EQUAL(result[i].value,7*(i+1));
        }
    }

    {
        std::vector<DataType<int,int> > result;
        hdb.getValueBetween(result, 100,299);
        BOOST_CHECK_EQUAL(result.size(), 200U);
        for(int i=0; i<200; i++) {
            BOOST_CHECK_EQUAL(result[i].key,i+100);
            BOOST_CHECK_EQUAL(result[i].value,7*(i+100));
        }
    }

    {
        std::vector<DataType<int,int> > result;
        hdb.getValueBetween(result, 100,899);
        BOOST_CHECK_EQUAL(result.size(), 800U);
        for(int i=0; i<800; i++) {
            BOOST_CHECK_EQUAL(result[i].key,i+100);
            BOOST_CHECK_EQUAL(result[i].value,7*(i+100));
        }
    }

    hdb.close();
}


BOOST_AUTO_TEST_SUITE_END()
