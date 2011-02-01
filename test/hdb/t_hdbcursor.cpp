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

#include <util/ustring/UString.h>

#include <hdb/HugeDB.h>


using namespace std;
using namespace izenelib::util;
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

    {
        std::cout << "Test seq forward to the end" << std::endl;
        int k,v;
        int count = 0;
        HDBCursor cursor = hdb.get_first_locn();
        while(hdb.get(cursor,k,v))
        {
            BOOST_CHECK_EQUAL(k,count);
            BOOST_CHECK_EQUAL(v,7*count);
            hdb.seq(cursor);
            count ++;
        }
        BOOST_CHECK_EQUAL(count, 1000);
    }

    {
        std::cout << "Test search to a position then seq forward" << std::endl;
        int k,v;
        int count = 800;
        HDBCursor cursor = hdb.search(800);
        while(hdb.get(cursor,k,v))
        {
            BOOST_CHECK_EQUAL(k,count);
            BOOST_CHECK_EQUAL(v,7*count);
            hdb.seq(cursor);
            count ++;
        }
        BOOST_CHECK_EQUAL(count, 1000);
    }

    hdb.close();
}

BOOST_AUTO_TEST_CASE(hdb_seq_backward)
{

    HDBType hdb("t_hdb_seq_backward");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, 7*i);

    {
        std::cout << "Test seq backward at first cursor" << std::endl;
        int k,v;
        HDBCursor cursor = hdb.get_first_locn();
        BOOST_CHECK_EQUAL(hdb.seq(cursor, ESD_BACKWARD), false);
        BOOST_CHECK_EQUAL(hdb.get(cursor, k , v), false);
    }

    {
        std::cout << "Test seq forward then seq backward" << std::endl;

        HDBCursor cursor = hdb.get_first_locn();
        for(int i=0; i<800; i++)
            hdb.seq(cursor, ESD_FORWARD);
        //500
        for(int i=0; i<500; i++)
            hdb.seq(cursor, ESD_BACKWARD);
        //300
        int k,v;
        hdb.get(cursor,k,v);
        BOOST_CHECK_EQUAL(k,300);
    }

    {
        std::cout << "Test seq forward to the end then seq backward to the end" << std::endl;
        HDBCursor cursor = hdb.get_first_locn();
        while(hdb.seq(cursor, ESD_FORWARD));

        int count = 999;
        while(hdb.seq(cursor, ESD_BACKWARD))
        {
            int k,v;
            hdb.get(cursor,k,v);
            BOOST_CHECK_EQUAL(k,count);
            BOOST_CHECK_EQUAL(v,7*count);
            count --;
        }
        BOOST_CHECK_EQUAL(count, -1);
    }

    {
        std::cout << "Test search to a position then seq backward" << std::endl;
        int count = 799;
        HDBCursor cursor = hdb.search(800);
        while(hdb.seq(cursor, ESD_BACKWARD))
        {
            int k,v;
            hdb.get(cursor,k,v);
            BOOST_CHECK_EQUAL(k,count);
            BOOST_CHECK_EQUAL(v,7*count);
            count --;
        }
        BOOST_CHECK_EQUAL(count, -1);
    }


    hdb.close();
}


BOOST_AUTO_TEST_CASE(hdb_search_forward)
{
    std::cout << "Test search forward" << std::endl;
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
        HDBCursor cursor = hdb.search(999);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), true);
        BOOST_CHECK_EQUAL(k, 999);
        BOOST_CHECK_EQUAL(v, 999*7);
    }

    {
        HDBCursor cursor = hdb.search(1000);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), false);
    }

    hdb.close();
}



BOOST_AUTO_TEST_CASE(hdb_search_backward)
{
    std::cout << "Test search backward" << std::endl;
    HDBType hdb("t_hdb_search_backward");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, 7*i);

    int k,v;

    {
        HDBCursor cursor = hdb.search(-1, ESD_BACKWARD);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), false);
    }

    {
        HDBCursor cursor = hdb.search(0, ESD_BACKWARD);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), true);
        BOOST_CHECK_EQUAL(k, 0);
        BOOST_CHECK_EQUAL(v, 0);
    }

    {
        HDBCursor cursor = hdb.search(100, ESD_BACKWARD);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), true);
        BOOST_CHECK_EQUAL(k, 100);
        BOOST_CHECK_EQUAL(v, 700);
    }

    {
        HDBCursor cursor = hdb.search(800, ESD_BACKWARD);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), true);
        BOOST_CHECK_EQUAL(k, 800);
        BOOST_CHECK_EQUAL(v, 5600);
    }

    {
        HDBCursor cursor = hdb.search(999, ESD_BACKWARD);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), true);
        BOOST_CHECK_EQUAL(k, 999);
        BOOST_CHECK_EQUAL(v, 999*7);
    }

    {
        HDBCursor cursor = hdb.search(1000, ESD_BACKWARD);
        BOOST_CHECK_EQUAL(hdb.get(cursor,k,v), true);
        BOOST_CHECK_EQUAL(k, 999);
        BOOST_CHECK_EQUAL(v, 999*7);
    }

    hdb.close();
}


BOOST_AUTO_TEST_CASE(hdb_getNext)
{
    std::cout << "Test getNext" << std::endl;
    HDBType hdb("t_hdb_getnext");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, 7*i);

    int nxtKey;
    BOOST_CHECK_EQUAL(hdb.getNext(-1, nxtKey), true);
    BOOST_CHECK_EQUAL(nxtKey, 0);
    for(int i=0; i<999;i++) {
        BOOST_CHECK_EQUAL(hdb.getNext(i, nxtKey), true);
        BOOST_CHECK_EQUAL(nxtKey, i+1);
    }
    BOOST_CHECK_EQUAL(hdb.getNext(999, nxtKey), false);
    BOOST_CHECK_EQUAL(hdb.getNext(1000, nxtKey), false);
}

BOOST_AUTO_TEST_CASE(hdb_getPrev)
{
    std::cout << "Test getPrev" << std::endl;
    HDBType hdb("t_hdb_getprev");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, 7*i);

    int prevKey;
    BOOST_CHECK_EQUAL(hdb.getPrev(1000, prevKey), true);
    BOOST_CHECK_EQUAL(prevKey, 999);
    for(int i=1; i<1000;i++) {
        BOOST_CHECK_EQUAL(hdb.getPrev(i, prevKey), true);
        BOOST_CHECK_EQUAL(prevKey, i-1);
    }
    BOOST_CHECK_EQUAL(hdb.getPrev(-1, prevKey), false);
    BOOST_CHECK_EQUAL(hdb.getPrev(0, prevKey), false);
}

BOOST_AUTO_TEST_CASE(hdb_getvalueforward)
{
    std::cout << "Test getvalueforward" << std::endl;
    HDBType hdb("t_hdb_getvalueforward");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, 7*i);

    {
        std::vector<DataType<int,int> > result;
        hdb.getValueForward(10,result,800);
        BOOST_CHECK_EQUAL(result.size(), 10U);
        for(int i=0; i<10; i++) {
            BOOST_CHECK_EQUAL(result[i].key,i+800);
        }
    }


    {
        std::vector<DataType<int,int> > result;
        hdb.getValueForward(1000,result,800);
        BOOST_CHECK_EQUAL(result.size(), 200U);
        for(int i=0; i<200; i++) {
            BOOST_CHECK_EQUAL(result[i].key,i+800);
        }
    }

}


BOOST_AUTO_TEST_CASE(hdb_getvaluebackward)
{
    std::cout << "Test getvaluebackward" << std::endl;
    HDBType hdb("t_hdb_getvaluebackward");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100U);
    hdb.open();
    for(int i=0; i<1000; i++)
        hdb.insertValue(i, 7*i);

    {
        std::vector<DataType<int,int> > result;
        hdb.getValueBackward(10,result,800);
        BOOST_CHECK_EQUAL(result.size(), 10U);
        for(int i=0; i<10; i++) {
            BOOST_CHECK_EQUAL(result[i].key,800-i);
        }
    }


    {
        std::vector<DataType<int,int> > result;
        hdb.getValueBackward(1000,result,800);
        BOOST_CHECK_EQUAL(result.size(), 801U);
        for(int i=0; i<801; i++) {
            BOOST_CHECK_EQUAL(result[i].key,800-i);
        }
    }

}

BOOST_AUTO_TEST_CASE(hdb_getvaluebetween)
{

    std::cout << "Test getvaluebetween" << std::endl;
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
