#include <util/ClockTimer.h>

#include <boost/test/unit_test.hpp>

#include <util/compressed_vector/VIntVector.h>

#include <string>
#include <iostream>
#include <algorithm>

using namespace izenelib::util;
using namespace std;

static unsigned *int_data;
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

BOOST_AUTO_TEST_SUITE(Compressed_vector_test)

BOOST_AUTO_TEST_CASE(ordered_vector1)
{
    izenelib::util::mem_pool memPool("1.data");
    izenelib::util::compressed_vector::OrderedVIntVector v(&memPool);

    size_t count = 1000;
    size_t i = 1;
    size_t magic = 3;
    for(; i <= count; ++i)
        v.push_back(i*magic);

    BOOST_CHECK(v.size() == count);

    typedef izenelib::util::compressed_vector::OrderedVIntVector::iterator IteratorType;
    IteratorType iter = v.begin();
    IteratorType end = v.end();
    i = 1;
    for(;iter !=end; ++iter,++i)
    {
        BOOST_CHECK(*iter == i*magic);
    }
    const izenelib::util::compressed_vector::OrderedVIntVector& vv(v);
    typedef izenelib::util::compressed_vector::OrderedVIntVector::const_iterator CIteratorType;
    CIteratorType citer = vv.begin();
    CIteratorType cend = vv.end();
    i = 1;
    for(;citer !=cend; ++citer,++i)
    {
        BOOST_CHECK(*citer == i*magic);
    }
}

BOOST_AUTO_TEST_CASE(ordered_vector2)
{
    izenelib::util::mem_pool memPool("1.data");
    izenelib::util::compressed_vector::OrderedVIntVector v(&memPool);

    unsigned data_size = 1024;

    init_large_and_sorted_data(data_size);

    for(unsigned i = 0; i < data_size; ++i)
        v.push_back(int_data[i]);
    BOOST_CHECK(v.size() == data_size);

    typedef izenelib::util::compressed_vector::OrderedVIntVector::iterator IteratorType;
    IteratorType iter = v.begin();
    IteratorType end = v.end();
    for(unsigned i = 0;iter !=end; ++iter,++i)
    {
        BOOST_CHECK_EQUAL(*iter,int_data[i]);		
    }

    delete [] int_data;
}


BOOST_AUTO_TEST_CASE(unordered_vector1)
{
    izenelib::util::mem_pool memPool("1.data");
    izenelib::util::compressed_vector::UnOrderedVIntVector v(&memPool);

    size_t count = 1000;
    size_t i = 1;
    size_t magic = 3;

    for(; i <=count; ++i)
        v.push_back(i*magic);

    BOOST_CHECK(v.size() == count);

    typedef izenelib::util::compressed_vector::UnOrderedVIntVector::iterator IteratorType;
    IteratorType iter = v.begin();
    IteratorType end = v.end();
    i = 1;
    for(;iter !=end; ++iter,++i)
    {
        BOOST_CHECK(*iter == i*magic );
    }
}

BOOST_AUTO_TEST_CASE(unordered_vector2)
{
    izenelib::util::mem_pool memPool("1.data");
    izenelib::util::compressed_vector::UnOrderedVIntVector v(&memPool);

    unsigned data_size = 1024;

    init_large_and_unsorted_data(data_size);

    for(unsigned i = 0; i < data_size; ++i)
        v.push_back(int_data[i]);
    BOOST_CHECK(v.size() == data_size);

    typedef izenelib::util::compressed_vector::UnOrderedVIntVector::iterator IteratorType;
    IteratorType iter = v.begin();
    IteratorType end = v.end();
    for(unsigned i = 0;iter !=end; ++iter,++i)
    {
        BOOST_CHECK_EQUAL(*iter,int_data[i]);		
    }

    delete [] int_data;
}

BOOST_AUTO_TEST_SUITE_END() // Compressed_vectortest

