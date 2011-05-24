#include <leveldb/db.h>
#include <am/leveldb/Table.h>

#include <util/Int2String.h>

#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

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

BOOST_AUTO_TEST_SUITE( t_leveldb )

BOOST_AUTO_TEST_CASE(index)
{
    bfs::remove_all(HOME_STR);
/*
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "leveldb", &db);
    BOOST_CHECK(status.ok());

    std::string keystr("key");
    leveldb::Slice key(keystr);
    std::string valuestr("value");
    leveldb::Slice value(valuestr);
    db->Put(leveldb::WriteOptions(), key, value);
*/
    init_data();

    izenelib::am::leveldb::Table<int, int> table(HOME_STR);
    table.open();
    int size = 100;
    int i;
    for (i = 1; i < size; ++i) {
        //Int2String key(i);
        //table.insert(key,int_data[i]);
        table.insert(i,i*100);
    }
    cout<<"insert finished"<<endl;

    for (i = 1; i < size; ++i) {
        //Int2String key(i);
        int value;
        table.get(i, value);
    //if(value != int_data[i])
	   //cout<<"i "<<i<<" value "<<value<<" data "<<int_data[i]<<endl;
	cout<<"i "<<i<<" value "<<value<<endl;			 
    }
    table.flush();

    destroy_data();
}

BOOST_AUTO_TEST_SUITE_END()

