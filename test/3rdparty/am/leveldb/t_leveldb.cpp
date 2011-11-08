#include <leveldb/db.h>
#include <am/leveldb/Table.h>

#include <util/Int2String.h>

#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#include <cstdlib>   // for rand()
#include <cctype>    // for isalnum()   
#include <algorithm> // for back_inserter


#include <boost/timer.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace leveldb;
namespace bfs = boost::filesystem;

static int data_size = 1000000;
static unsigned *int_data;

void init_data()
{
    int i;
    std::cout<<"generating data... "<<std::endl;
    srand48(11);
    int_data = (unsigned*)calloc(data_size, sizeof(unsigned));
    for (i = 0; i < data_size; ++i) {
        int_data[i] = (unsigned)(data_size * drand48() / 4) * 271828183u;
    }
    std::cout<<"done!\n";
}

void destroy_data()
{
    free(int_data);
}

const char* HOME_STR = "leveldb";

char rand_alnum()
{
    char c;
    while (!std::isalnum(c = static_cast<char>(std::rand()))) ;
    return c;
}


std::string rand_alnum_str (std::string::size_type sz)
{
    std::string s;
    s.reserve  (sz);
    generate_n (std::back_inserter(s), sz, rand_alnum);
    return s;
}

BOOST_AUTO_TEST_SUITE( t_leveldb )

BOOST_AUTO_TEST_CASE(raw_index)
{
    bfs::remove_all(HOME_STR);

    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "leveldb", &db);
    BOOST_CHECK(status.ok());

    for (int i = 1; i < 100; ++i) {
        std::string keystr = rand_alnum_str(4);
        leveldb::Slice key(keystr);
        std::string valuestr = rand_alnum_str(10);
        leveldb::Slice value(valuestr);
        db->Put(leveldb::WriteOptions(), key, value);
    }
    leveldb::Slice property("leveldb.stats");
    std::string propertyValue;
    db->GetProperty(property,  &propertyValue);
    cout<<propertyValue<<endl;
    leveldb::ReadOptions itOptions;
    itOptions.fill_cache = false;
    leveldb::Iterator* dbIt = db->NewIterator(itOptions);
    dbIt->SeekToFirst();
    delete dbIt;
    delete db;
}


BOOST_AUTO_TEST_SUITE_END()

