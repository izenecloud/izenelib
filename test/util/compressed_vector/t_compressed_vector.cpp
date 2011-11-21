#include <util/ClockTimer.h>

#include <boost/test/unit_test.hpp>

#include <util/compressed_vector/VIntVector.h>

#include <string>
#include <iostream>
#include <algorithm>

using namespace izenelib::util;
using namespace std;


BOOST_AUTO_TEST_SUITE(Compressed_vector_test)

BOOST_AUTO_TEST_CASE(ordered_vector)
{
    MemPool memPool(128);
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

BOOST_AUTO_TEST_CASE(unordered_vector)
{
    MemPool memPool(128);
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

BOOST_AUTO_TEST_SUITE_END() // Compressed_vectortest

