#include <string>

#include <boost/test/unit_test.hpp>

#include <sdb/SequentialDB.h>
#include <util/ProcMemInfo.h>
#include <am/sdb_trie_v1/sdb_hash2btree.h>

using namespace std;
using namespace izenelib::am;
using namespace izenelib::sdb;

typedef unordered_sdb_fixed<int, int> Sdb1;
typedef ordered_sdb<int, int> Sdb2;

// pseudo random number generator
inline int myrand(void) {
	static int cnt = 0;
	return (lrand48() + cnt++) & 0x7FFFFFFF;
}

void displayMemInfo(std::ostream& os = std::cout) {
	unsigned long rlimit = 0, vm = 0, rss = 0;
	ProcMemInfo::getProcMemInfo(vm, rss, rlimit);
	os << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;
}

BOOST_AUTO_TEST_SUITE( sdb_trie_suite )

BOOST_AUTO_TEST_CASE(conv_sdb_hash2btree_test)
{
    Sdb1 sdb1("conv_sdb_hash2btree_test.hash.sdb");
    sdb1.open();
    for(int i =0; i<100; i++ )
        BOOST_CHECK_EQUAL( sdb1.insertValue(i,i), true);
    sdb1.flush();

    Sdb2 sdb2("conv_sdb_hash2btree_test.btree.sdb");
    sdb2.open();
    Conv_Sdb_Hash2Btree<Sdb1, Sdb2> converter(sdb1, sdb2, "conv_sdb_hash2btree_test");
    converter.startThread();
    converter.joinThread();

    BOOST_CHECK_EQUAL(sdb2.numItems(), 100);
    for(int i=0; i<100; i++ )
    {
        int r;
        BOOST_CHECK_EQUAL( sdb2.getValue(i, r), true );
        BOOST_CHECK_EQUAL(r, i);
    }

    remove("conv_sdb_hash2btree_test.hash.sdb");
    remove("conv_sdb_hash2btree_test.btree.sdb");
}

//#define TEST_BENCH

#ifdef TEST_BENCH
BOOST_AUTO_TEST_CASE(conv_sdb_hash2btree_bench)
{
    {
        Sdb1 sdb1("conv_sdb_hash2btree_bench.hash.sdb");
        sdb1.setBucketSize(512);
        sdb1.setCacheSize(256*1024);
        sdb1.setDegree(16);
        sdb1.open();
        for(int i =0; i<5000000; i++ )
        {
            if(i%1000000 == 0) {
                std::cout << "insert " << i << " elements" << std::endl;
                displayMemInfo();
            }
            sdb1.insertValue(i,i);
        }
        sdb1.flush();

        Sdb2 sdb2("conv_sdb_hash2btree_bench.btree.sdb");
        sdb2.setPageSize(2048);
        sdb2.setCacheSize(16*1024);
        sdb2.setDegree(24);
        sdb2.open();
        Conv_Sdb_Hash2Btree<Sdb1, Sdb2> converter(sdb1, sdb2, "conv_sdb_hash2btree_bench");

        std::cout << "begin convert" << std::endl;
        converter.startThread();
        converter.joinThread();
        std::cout << "finish convert" << std::endl;

        BOOST_CHECK_EQUAL(sdb2.numItems(), 5000000);
        remove("conv_sdb_hash2btree_bench.hash.sdb");
        remove("conv_sdb_hash2btree_bench.btree.sdb");
    }

    {
        Sdb2 sdb2("conv_sdb_hash2btree_bench.btree.example.sdb");
        sdb2.setPageSize(2048);
        sdb2.setCacheSize(128*1024);
        sdb2.setDegree(24);
        sdb2.open();
        for(int i =0; i<5000000; i++ )
        {
            if(i%1000000 == 0) {
                std::cout << "insert " << i << " elements" << std::endl;
                displayMemInfo();
            }
            sdb2.insertValue(myrand(),i);
        }

        BOOST_CHECK_EQUAL(sdb2.numItems(), 5000000);
        remove("conv_sdb_hash2btree_bench.btree.example.sdb");
    }
}
#endif

BOOST_AUTO_TEST_SUITE_END()

