#include <3rdparty/am/google/sparse_hash_map>

#include <util/Int2String.h>
#include <util/izene_serialization.h>

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
using google::sparse_hash_map;      // namespace where class lives by default
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

const char* HOME_STR = "sparsehash";

typedef sparse_hash_map<int, int> SParseType;
MAKE_FEBIRD_SERIALIZATION(SParseType )

BOOST_AUTO_TEST_SUITE( t_sparse )

BOOST_AUTO_TEST_CASE(index)
{
    bfs::remove_all(HOME_STR);

    init_data();

    int size = 10000;

    sparse_hash_map<int, int> table;
    for (int i = 0; i < size; ++i)
    {
        table[i] = int_data[i];
    }

    for (int i = 0; i < size; ++i)
    {
        BOOST_CHECK_EQUAL(table[i] , int_data[i]);
    }
    FILE* pf = fopen ( HOME_STR, "wb" );
    table.write_metadata ( pf );
    table.write_nopointer_data ( pf );
    fclose ( pf );


    {
        sparse_hash_map<int, int> readMap;
        FILE* f =fopen ( HOME_STR, "rb" );
        readMap.read_metadata ( f );
        readMap.read_nopointer_data ( f );
        fclose ( f );

        for (int i = 0; i < size; ++i)
        {
            BOOST_CHECK_EQUAL(readMap[i] , int_data[i]);
        }

    }

    {
        izenelib::util::izene_serialization_febird<sparse_hash_map<int, int> > isb(table);
        char* ptr;
        size_t sz;
        isb.write_image(ptr, sz);
        sparse_hash_map<int, int> readMap;

        izenelib::util::izene_deserialization_febird<sparse_hash_map<int, int> > idb(ptr, sz);
        idb.read_image(readMap);

        for (int i = 0; i < size; ++i)
        {
            BOOST_CHECK_EQUAL(readMap[i] , int_data[i]);
        }

    }
#if 0
    izenelib::util::MemPool memPool(1024*1024);
    izenelib::util::BlockAlloc blockAlloc(memPool);
    izenelib::util::RegionAlloc regionAlloc(blockAlloc);

    typedef izenelib::util::MemPoolAllocator<pair<const int, int> > SparseAlloc;

    SparseAlloc alloc(regionAlloc);

    typedef sparse_hash_map<int,int,SPARSEHASH_HASH<int>,STL_NAMESPACE::equal_to<int>,SparseAlloc > SparseHashType;

    SparseHashType hashmap(0, SparseHashType::hasher(), SparseHashType::key_equal(), alloc);
    for (int i = 0; i < size; ++i)
    {
        hashmap[i] = int_data[i];
    }

    for (int i = 0; i < size; ++i)
    {
        BOOST_CHECK_EQUAL(hashmap[i] , int_data[i]);
    }

#endif
    destroy_data();

}

BOOST_AUTO_TEST_SUITE_END()
