#include <util/compression/int/compressor.h>
#include <util/compression/int/pfordelta_mix_compressor.h>
#include <util/compression/int/VSE-R.hpp>

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
const uint32_t data_size = 1024;


void init_large_and_sorted_data(int data_size)
{
    std::cout<<"generating data... "<<std::endl;
    int MAXID = 35;
    MAXID *= 100000;

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

void init_small_and_sorted_data(int data_size)
{
    std::cout<<"generating data... "<<std::endl;
    int MAXID = 2000;

    int_data = new unsigned[data_size];
    srand( (unsigned)time(NULL) );

    for (int i = 0; i < data_size; ++i) {
        int_data[i] = rand() % MAXID;
    }
    std::sort(int_data, int_data+ data_size);
    for(int i = data_size - 1; i > 0; --i) {
        int_data[i] -= int_data[i-1];
        if (int_data[i] == 0) ++int_data[i];
    }

    std::cout<<"done!\n";
}


void init_small_and_unsorted_data(int data_size)
{
    std::cout<<"generating data... "<<std::endl;
    int MAXID = 1000;

    int_data = new unsigned[data_size];
    srand( (unsigned)time(NULL) );

    for (int i = 0; i < data_size; ++i) {
        int_data[i] = rand() % MAXID;
    }

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

template<class T>
void test_compressor(Compressor<T>& compressor, std::string info){
    cout<<info<<": "<<endl;
    unsigned int * compresseddata = new unsigned int[data_size];
    unsigned int * decompresseddata = new unsigned int[data_size * 2];
    memset(compresseddata,0,data_size*sizeof(unsigned));

    init_small_and_sorted_data(data_size);
    izenelib::util::ClockTimer timer_small;
    int compressNum = compressor.compress(int_data, compresseddata, data_size);
    cout<<"Small_sorted_data, compress, length: "<<compressNum<<" ,time elapsed: "<<timer_small.elapsed()<<endl;
    BOOST_CHECK_LE(compressNum, data_size);
    int decompressNum = compressor.decompress(compresseddata, decompresseddata, data_size);
    BOOST_CHECK_EQUAL(decompressNum, compressNum);
    cout<<"Small_sorted_data, decompress, length:  "<<decompressNum<<" ,time elapsed: "<<timer_small.elapsed()<<endl;
    for(uint32_t i = 0; i < data_size; ++i){
        BOOST_CHECK_EQUAL(int_data[i],decompresseddata[i]);
        if(int_data[i] != decompresseddata[i])
            break;
    }
    delete[] int_data;

    init_large_and_sorted_data(data_size);
    memset(compresseddata,0,data_size*sizeof(unsigned));
    izenelib::util::ClockTimer timer_large;
    compressNum = compressor.compress(int_data, compresseddata, data_size);
    cout<<"Large_sorted_data, compress, length: "<<compressNum<<" ,time elapsed: "<<timer_large.elapsed()<<endl;
    BOOST_CHECK_LE(compressNum, data_size);
    decompressNum = compressor.decompress(compresseddata, decompresseddata, data_size);
    BOOST_CHECK_EQUAL(decompressNum, compressNum);
    cout<<"Large_sorted_data, decompress, length:  "<<decompressNum<<" ,time elapsed: "<<timer_large.elapsed()<<endl;
    for(uint32_t i = 0; i < data_size; ++i){
        BOOST_CHECK_EQUAL(int_data[i],decompresseddata[i]);
        if(int_data[i] != decompresseddata[i])
            break;
    }
   delete[] int_data;


    cout<<endl;
    delete[] decompresseddata;
    delete[] compresseddata;

}

BOOST_AUTO_TEST_CASE(VSE_R_test)
{
    cout<<"VSE_R: "<<endl;
    uint32_t * compressed_data = new uint32_t[data_size];
    unsigned * decompressed_data = new uint32_t[data_size*2];
    uint32_t nvalue;

    init_small_and_sorted_data(data_size);
    izenelib::util::ClockTimer timer_small;
    VSE_R::encodeArray(int_data, data_size, compressed_data, nvalue);
    cout<<"Small_sorted_data, compress, length: "<<nvalue<<", time elapsed: "<<timer_small.elapsed()<<endl;
    BOOST_CHECK(nvalue < data_size);
    VSE_R::decodeArray(compressed_data, nvalue, decompressed_data, data_size);
    cout<<"Small_sorted_data, decompress, length: "<<nvalue<<", time elapsed: "<<timer_small.elapsed()<<endl;
    for(uint32_t i = 0; i < data_size; ++i){
       BOOST_CHECK_EQUAL(int_data[i],decompressed_data[i]);
       if(int_data[i] != decompressed_data[i])
           break;
    }
    delete[] int_data;

    init_large_and_sorted_data(data_size);
    izenelib::util::ClockTimer timer_large;
    VSE_R::encodeArray(int_data, data_size, compressed_data, nvalue);
    cout<<"Large_sorted_data, compress, length: "<<nvalue<<", time elapsed: "<<timer_large.elapsed()<<endl;
    BOOST_CHECK(nvalue < data_size);
    VSE_R::decodeArray(compressed_data, nvalue, decompressed_data, data_size);
    cout<<"Large_sorted_data, decompress, length: "<<nvalue<<", time elapsed: "<<timer_large.elapsed()<<endl;
    for(uint32_t i = 0; i < data_size; ++i){
       BOOST_CHECK_EQUAL(int_data[i],decompressed_data[i]);
       if(int_data[i] != decompressed_data[i])
           break;
    }
    delete[] int_data;

    cout<<endl;
    delete[] compressed_data;
    delete[] decompressed_data;
}

BOOST_AUTO_TEST_CASE(compressor)
{
    S16_Compressor s16;
    test_compressor(s16, "S16_Compressor");
    VByte_Compressor vbyte;
    test_compressor(vbyte, "VByte_Compressor");
    PForDelta_Compressor pfor;
    test_compressor(pfor, "PForDelta_Compressor");
    PForDeltaMix_Compressor pformix;
    test_compressor(pformix, "PForDeltaMix_Compressor");
    PForDeltaMixS16_Compressor pformix16;
    test_compressor(pformix16, "PForDeltaMixS16_Compressor");
    PForDeltaMixS9_Compressor pformixs9;
    test_compressor(pformixs9, "PForDeltaMixS9_Compressor");
    FastPFor_Compressor fastpfor;
    test_compressor(fastpfor, "FastPFor_Compressor");
    SIMDFastPFor_Compressor simdfastpfor;
    test_compressor(simdfastpfor, "SIMDFastPFor_Compressor");
    Simple8b_Compressor simple8b;
    test_compressor(simple8b, "Simple8b_Compressor");
}
