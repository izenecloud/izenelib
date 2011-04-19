

#include <string>
#include <stdlib.h>
#include <sys/time.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>

#include <map>
#include <3rdparty/am/babudb/babudb.h>
#include <3rdparty/am/babudb/profiles/string_key.h>
#include <3rdparty/am/babudb/index/merger.h>
#include "babudb/profiles/string_db.h"


#include <3rdparty/am/babudb/platform/memory_mapped_file.h>

using YIELD::MemoryMappedFile;
using namespace babudb;
using namespace std;

#include <boost/timer.hpp>
#include <boost/test/unit_test.hpp>

#include <assert.h>
using namespace std;

BOOST_AUTO_TEST_SUITE( t_babudb )

BOOST_AUTO_TEST_CASE(index)
{
/*
    StringOrder myorder;
    std::vector<babudb::IndexDescriptor> indices;
    indices.push_back(std::make_pair("testidx", &myorder));
    Database* db = Database::Open("babudb", indices);

    StringSetOperation(1, "testidx", "Key1","data1").ApplyTo(*db, 1);
    StringSetOperation(2, "testidx", "Key2","data2").ApplyTo(*db, 2);

    BOOST_CHECK(! db->Lookup("testidx",DataHolder("Key1")).isEmpty());

    StringSetOperation(3, "testidx", "Key1").ApplyTo(*db, 3);

    BOOST_CHECK(db->Lookup("testidx",DataHolder("Key1")).isEmpty());
*/
    vector<string> indices; indices.push_back("index");
    StringDB* db = StringDB::Open("babudb", indices);
	
	// now you can use the database and change it atomically
	//db->Remove("index", "key4");
	//db->Commit();
	

    int N = 100000;
    char key[100];
    char data[100];
    for (int i = 0; i < N; ++i)
    {
        unsigned intdata = (N * drand48() / 4) * 271828183u;
        sprintf(key, "%x", intdata);
        sprintf(data, "%x", i); 
        //StringSetOperation(1, "testidx", key,data).ApplyTo(*db, 1);
        db->Add("index", key,data);		
    }

//    vector<pair<string, babudb::lsn_t> > index_versions = db->GetIndexVersions();

//    for (vector<pair<string, babudb::lsn_t> >::iterator i = index_versions.begin(); i != index_versions.end(); ++i) {
//        db->Snapshot(i->first);
//        db->CompactIndex(i->first, db->GetCurrentLSN());
//    }

    delete db;
}

BOOST_AUTO_TEST_SUITE_END()

