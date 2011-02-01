/// @file   t_hdb.cpp
/// @brief  A test unit for checking hdb's correctness.
/// @author Wei Cao
/// @date   2000-08-14
///
///
/// @details
///


#include <string>

#include <boost/test/unit_test.hpp>

#include <util/ustring/UString.h>

#include <hdb/HugeDB.h>


using namespace std;
using namespace izenelib::util;
using namespace izenelib::am;
using namespace izenelib::hdb;

BOOST_AUTO_TEST_SUITE( hdb_suite )


BOOST_AUTO_TEST_CASE(hdb_insert)
{
    ordered_hdb<int, int> hdb("t_hdb_insert");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, i);
    hdb.optimize();
    BOOST_CHECK_EQUAL(hdb.numItems(), 1000U);
    for(int i=0; i<1000; i++)
    {
        int r = -1;
        BOOST_CHECK_EQUAL(hdb.getValue(i, r), true);
        BOOST_CHECK_EQUAL(r, i);
    }
    hdb.close();
}

BOOST_AUTO_TEST_CASE(hdb_insert_merge0)
{
    ordered_hdb<int, int> hdb("t_hdb_insert_merge0");
    hdb.setMergeFactor(4);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<100; i++)
        hdb.insertValue(i, i);
    for(int i=0; i<100; i++)
        hdb.insertValue(i, i);
    for(int i=0; i<100; i++)
        hdb.insertValue(i, i);
    for(int i=0; i<100; i++)
        hdb.insertValue(i, i);
    for(int i=0; i<100; i++)
        hdb.insertValue(i, i);
    for(int i=0; i<100; i++)
        hdb.insertValue(i, i);
    hdb.optimize();
    BOOST_CHECK_EQUAL(hdb.numItems(), 100U);
    hdb.close();
}


BOOST_AUTO_TEST_CASE(hdb_insert_merge1)
{
    ordered_hdb<int, int> hdb("t_hdb_insert_merge1");
    hdb.setMergeFactor(4);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<189; i++)
        hdb.insertValue(i, i);
    for(int i=0; i<189; i++)
        hdb.insertValue(i, i);
    for(int i=0; i<189; i++)
        hdb.insertValue(i, i);
    for(int i=0; i<189; i++)
        hdb.insertValue(i, i);
    hdb.optimize();
    BOOST_CHECK_EQUAL(hdb.numItems(), 189U);
    hdb.close();
}

BOOST_AUTO_TEST_CASE(hdb_update0)
{
    ordered_hdb<int, int> hdb("t_hdb_update0");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, i);
    for(int i=0; i<1000; i++)
        hdb.update(i, 2*i);
    hdb.optimize();
    BOOST_CHECK_EQUAL(hdb.numItems(), 1000U);
    for(int i=0; i<1000; i++)
    {
        int r = -1;
        BOOST_CHECK_EQUAL(hdb.getValue(i, r), true);
        BOOST_CHECK_EQUAL(r, 2*i);
    }
    hdb.close();
}


BOOST_AUTO_TEST_CASE(hdb_update1)
{
    ordered_hdb<int, int> hdb("t_hdb_update1");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++) {
        hdb.insertValue(i, i);
        hdb.update(i, 2*i);
    }
    hdb.optimize();
    BOOST_CHECK_EQUAL(hdb.numItems(), 1000U);
    for(int i=0; i<1000; i++)
    {
        int r = -1;
        BOOST_CHECK_EQUAL(hdb.getValue(i, r), true);
        BOOST_CHECK_EQUAL(r, 2*i);
    }
    hdb.close();
}


BOOST_AUTO_TEST_CASE(hdb_delta0)
{
    ordered_hdb<int, int> hdb("t_hdb_delta0");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.update(i, i);
    for(int i=0; i<1000; i++)
        hdb.delta(i, i);
    hdb.optimize();
    BOOST_CHECK_EQUAL(hdb.numItems(), 1000U);
    for(int i=0; i<1000; i++)
    {
        int r = -1;
        BOOST_CHECK_EQUAL(hdb.getValue(i, r), true);
        BOOST_CHECK_EQUAL(r, 2*i);
    }
    hdb.close();
}


BOOST_AUTO_TEST_CASE(hdb_delta1)
{
    ordered_hdb<int, int> hdb("t_hdb_delta1");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++) {
        hdb.update(i, i);
        hdb.delta(i, i);
    }
    hdb.optimize();
    BOOST_CHECK_EQUAL(hdb.numItems(), 1000U);
    for(int i=0; i<1000; i++)
    {
        int r = -1;
        BOOST_CHECK_EQUAL(hdb.getValue(i, r), true);
        BOOST_CHECK_EQUAL(r, 2*i);
    }
    hdb.close();
}

BOOST_AUTO_TEST_CASE(hdb_delete)
{
    ordered_hdb<int, int> hdb("t_hdb_delete");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, i);
    for(int i=0; i<1000; i++)
        hdb.del(i);
    hdb.optimize();
    BOOST_CHECK_EQUAL(hdb.numItems(), 0U);
    hdb.close();
}

BOOST_AUTO_TEST_CASE(hdb_clear)
{
    ordered_hdb<int, int> hdb("t_hdb_clear");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, i);
    hdb.optimize();
    hdb.clear();
    BOOST_CHECK_EQUAL(hdb.numItems(), 0U);
    hdb.close();
}

BOOST_AUTO_TEST_CASE(hdb_release)
{
    ordered_hdb<int, int> hdb("t_hdb_clear");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, i);
    BOOST_CHECK_EQUAL(hdb.numItems(), 1000U);
    hdb.release();
    BOOST_CHECK_EQUAL(hdb.numItems(), 1000U);
    for(int i=0; i<1000; i++)
    {
        int r = -1;
        BOOST_CHECK_EQUAL(hdb.getValue(i, r), true);
        BOOST_CHECK_EQUAL(r, i);
    }
    hdb.close();

}


BOOST_AUTO_TEST_SUITE_END()
