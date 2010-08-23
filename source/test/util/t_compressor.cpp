#include <util/compression/int/compressor.h>
#include <util/compression/int/pfordelta_mix_compressor.h>

#include <util/ClockTimer.h>

#include <boost/test/unit_test.hpp>
#include <boost/timer.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <string>
#include <iostream>
#include <algorithm>

using namespace izenelib::util::compression;
using namespace std;

static unsigned *int_data;


void init_small_and_sorted_data(int data_size)
{
    std::cout<<"generating data... "<<std::endl;
    int MAXID = 35;
    MAXID *= 1000000;

    bool * mark = new bool[MAXID];
    for (int i = 0; i < MAXID; ++i) mark[i] = false;
	
    int_data = new unsigned[data_size];
    srand( (unsigned)time(NULL) );

    for (int i = 0; i < data_size; ++i) {
        int temp = rand() % MAXID;
        if (mark[temp] == false) {
        	mark[temp] = true;
        	int_data[i] = static_cast<uint32_t>(temp);
        }
        else {
        	--i;
        }
    }
    std::sort(int_data, int_data+ data_size);
    for(int i = data_size - 1; i > 0; --i) {
        int_data[i] -= int_data[i-1];
        if (int_data[i] == 0) ++int_data[i]; 
    }

    delete[] mark;
    std::cout<<"done!\n";
}

void init_large_and_unsorted_data(int data_size)
{
    std::cout<<"generating data... "<<std::endl;
    int_data = new unsigned[data_size];
    srand( (unsigned)time(NULL) );

    for (int i = 0; i < data_size; ++i) {
        int_data[i] = (unsigned)(data_size * drand48() / 4) * 271828183u;		
    }

    std::cout<<"done!\n";
}

BOOST_AUTO_TEST_CASE(vbyte_compressor_test)
{
    int data_size = 1024;
    init_small_and_sorted_data(data_size);
    unsigned * compressed_data = new unsigned[data_size];
    VByte_Compressor compressor;
    izenelib::util::ClockTimer timer;
    int retSize = compressor.compress(int_data, compressed_data, data_size);
    cout<<"ret size "<<retSize<<" time elapsed: "<<timer.elapsed()<<endl;	
    BOOST_CHECK(retSize < data_size);

    unsigned * decompressed_data = new unsigned[data_size];
    retSize = compressor.decompress(compressed_data, decompressed_data, data_size);
    cout<<"ret size "<<retSize<<" time elapsed: "<<timer.elapsed()<<endl;	

    for(int i = 0; i < data_size; ++i)
       BOOST_CHECK_EQUAL(int_data[i],decompressed_data[i]);

    delete[] compressed_data;
    delete[] decompressed_data;
    delete[] int_data;
}


BOOST_AUTO_TEST_CASE(pfordelta_mix_compressor_test)
{
    int data_size = 1024;
    init_small_and_sorted_data(data_size);

    unsigned * compressed_data = new unsigned[data_size];
    PForDeltaMix_Compressor compressor;
    izenelib::util::ClockTimer timer;
    int retSize = compressor.compress(int_data, (char *)compressed_data, data_size);
	
    cout<<"ret size "<<retSize<<" time elapsed: "<<timer.elapsed()<<endl;	
    BOOST_CHECK((retSize/4) < data_size);

    unsigned * decompressed_data = new unsigned[data_size];
    retSize = compressor.decompress((char *)compressed_data, decompressed_data, retSize);
	
    cout<<"ret size "<<retSize<<" time elapsed: "<<timer.elapsed()<<endl;	

    for(int i = 0; i < data_size; ++i)
       BOOST_CHECK_EQUAL(int_data[i],decompressed_data[i]);

    delete[] compressed_data;
    delete[] decompressed_data;
    delete[] int_data;
}


