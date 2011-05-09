#include <leveldb/db.h>

#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace leveldb;

BOOST_AUTO_TEST_SUITE( t_leveldb )

BOOST_AUTO_TEST_CASE(index)
{
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "leveldb", &db);
    BOOST_CHECK(status.ok());
}

BOOST_AUTO_TEST_SUITE_END()

