/*
 * =====================================================================================
 *
 *       Filename:  t_khashtable.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2013年05月21日 15时30分59秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  B5M.com
 *
 * =====================================================================================
 */
#include "am/hashtable/khash_table.hpp"

#include <boost/test/unit_test.hpp>
#include <stdlib.h>
#include <sys/time.h>

#include <string>
#include <time.h>
#include <math.h>

#include <sys/time.h>
#include <fstream>
#include <iostream>
#include <vector>
#include<stdio.h>

using namespace izenelib::am;
using namespace std;

uint32_t error_count = 0;

BOOST_AUTO_TEST_SUITE(khash_test)

BOOST_AUTO_TEST_CASE(integer_hash_test)
{
    const uint32_t scale = 1000;
    {
        KIntegerHashTable<uint32_t, uint64_t> t(scale/10, scale/30);
        for ( uint32_t i=0; i<scale; ++i)
            t.insert(i, (uint64_t)i+1);

        for ( uint32_t i=0; i<scale; ++i)
        {
            uint64_t* n = t.find(i);
            BOOST_CHECK(*n == i+1);
        }
        t.persistence("./tmp.khash");
    }
    {
        KIntegerHashTable<uint32_t, uint64_t> t(scale/10, scale/30);
        t.load("./tmp.khash");
        BOOST_CHECK(t.size() == scale);
        for ( uint32_t i=0; i<scale; ++i)
        {
            uint64_t* n = t.find(i);
            BOOST_CHECK(*n == i+1);
        }
        for ( uint32_t i=0; i<scale; i+=2)
            BOOST_CHECK(t.erase(i));
        for ( uint32_t i=0; i<scale; ++i)
        {
            uint64_t* n = t.find(i);
            if (i%2 == 0)
                BOOST_CHECK(n == NULL);
            else
                BOOST_CHECK(*n == i+1);
        }
        BOOST_CHECK(t.size() == scale/2);
        uint32_t h = 0;
        for ( KIntegerHashTable<uint32_t, uint64_t>::iterator it=t.begin(); it!=t.end(); ++it,++h)
        {
            BOOST_CHECK(*it.key() < scale);
            BOOST_CHECK(*it.value() < scale+1);
        }
        BOOST_CHECK(h == t.size());
    }
}

std::string tostring(uint32_t i)
{
    char buf[10];
    sprintf(buf, "%d", i);
    return std::string(buf);
}
BOOST_AUTO_TEST_CASE(string_hash_test)
{
    const uint32_t scale = 1000;
    {
        KStringHashTable<std::string, uint32_t> t(scale/10, scale/30);
        for ( uint32_t i=0; i<scale; ++i)
            t.insert(tostring(i), i+1);
        for ( uint32_t i=0; i<scale; ++i)
        {
            uint32_t* n = t.find(tostring(i));
            BOOST_CHECK(*n == i+1);
        }
        for ( uint32_t i=0; i<scale; i+=2)
            BOOST_CHECK(t.erase(tostring(i)));
        for ( uint32_t i=0; i<scale; ++i)
        {
            uint32_t* n = t.find(tostring(i));
            if (i%2 == 0)
                BOOST_CHECK(n == NULL);
            else
                BOOST_CHECK(*n == i+1);
        }
        BOOST_CHECK(t.size() == scale/2);
        uint32_t h = 0;
        for ( KStringHashTable<std::string, uint32_t>::iterator it=t.begin(); it!=t.end(); ++it,h++)
            BOOST_CHECK(*it.value() < scale+1);
        std::cout<<h<<std::endl;
        std::cout<<t.size()<<std::endl;
        BOOST_CHECK(h == t.size());
    }
}
BOOST_AUTO_TEST_SUITE_END()

