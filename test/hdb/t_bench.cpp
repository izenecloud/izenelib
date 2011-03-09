/// @file   t_sdb.cpp
/// @brief  A test unit for checking class ScalableDB's correctness.
/// @author Wei Cao
/// @date   2000-08-14
///
///
/// @details
///


#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/test/unit_test.hpp>

#include <util/ustring/UString.h>
#include <hdb/HugeDB.h>
#include <util/ProcMemInfo.h>

using namespace std;
using namespace izenelib::util;
using namespace izenelib::am;
using namespace izenelib::hdb;

// pseudo random number generator
inline int myrand(void) {
	static int cnt = 0;
	return (lrand48() + cnt++) & 0x7FFFFFFF;
}

void displayMemInfo(std::ostream& os = std::cout) {
	unsigned long vm = 0, rss = 0;
	ProcMemInfo::getProcMemInfo(vm, rss);
	os << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;
}

BOOST_AUTO_TEST_SUITE( sdb_bench_suite )

BOOST_AUTO_TEST_CASE(bench_insert)
{
    displayMemInfo();

    std::cout << "building hdb" << std::endl;
    //ordered_hdb_no_delta< izenelib::util::UString, izenelib::util::UString> hdb("bench0");
    //ordered_hdb_no_delta< std::string, std::string> hdb("bench0");

    ordered_hdb_no_delta< int, int> hdb("bench0");
    hdb.setMergeFactor(2);
    hdb.setCachedRecordsNumber(100000);
    hdb.open();

    boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
    for(int i =0; i<1000000; i++ )
    {
        if(i%100000 == 0) {
            std::cout << "insert " << i << " elements" << std::endl;
            displayMemInfo();
        }
    //	std::string k = boost::lexical_cast<std::string>(myrand() );
    //	std::string v = boost::lexical_cast<std::string>(myrand() );
    //	izenelib::util::UString key = k;
    //	izenelib::util::UString value = v;
        //hdb.insertValue(key,value);
        //hdb.insertValue(k,v);
        hdb.insertValue(myrand(),myrand());
    }
    hdb.close();

    boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - start;
    std::cout << "build hdb cost " << td.total_microseconds()/1000000 << " seconds" << std::endl;

    hdb.release();

    displayMemInfo();
}

//
//BOOST_AUTO_TEST_CASE(bench_find)
//{
//    std::cout << "find in unoptimized hdb" << std::endl;
//    ordered_hdb_fixed<int, int> hdb("bench0");
//    hdb.setMergeFactor(4);
//    hdb.setCachedRecordsNumber(5000000);
//    hdb.open();
//
//    boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
//    for(int i =0; i<10000; i++ )
//    {
//        if(i%1000 == 0) {
//            std::cout << "find " << i << " elements" << std::endl;
//            displayMemInfo();
//        }
//        hdb.getValue(myrand(), i);
//    }
//
//    hdb.close();
//    boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - start;
//    std::cout << "find in hdb cost " << td.total_microseconds()/1000000 << " seconds";
//}

//
//BOOST_AUTO_TEST_CASE(bench_find_after_optimized)
//{
//    std::cout << "find in unoptimized hdb" << std::endl;
//    ordered_hdb_fixed<int, int> hdb("bench0");
//    hdb.setMergeFactor(4);
//    hdb.setCachedRecordsNumber(5000000);
//    hdb.open();
//
//    hdb.optimize();
//    std::cout << hdb.numItems() << std::endl;
//
//    boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
//    for(int i =0; i<100000; i++ )
//    {
//        if(i%10000 == 0) {
//            std::cout << "find " << i << " elements" << std::endl;
//            displayMemInfo();
//        }
//        hdb.getValue(myrand(), i);
//    }
//    for(int i =0; i<1000; i++ )
//    {
//        if(i%100 == 0) {
//            std::cout << "find " << i << " elements" << std::endl;
//            displayMemInfo();
//        }
//        hdb.getValue(myrand(), i);
//    }
//
//    hdb.close();
//    boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - start_;
//    std::cout << "find in hdb cost " << td.micro_seconds()/1000000 << " seconds";
//}


BOOST_AUTO_TEST_SUITE_END()
