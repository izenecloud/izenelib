#include <util/ClockTimer.h>

#include <boost/test/unit_test.hpp>

#include <util/compressed_vector/OrderedVector.h>

#include <string>
#include <iostream>
#include <algorithm>

using namespace izenelib::util;
using namespace std;


BOOST_AUTO_TEST_SUITE(Compressed_vector_test)
BOOST_AUTO_TEST_CASE(ordered_vector)
{
    MemPool memPool(128);
    izenelib::util::compressed_vector::OrderedVector v(&memPool);

    size_t count = 1000;
    size_t i = 1;
    for(; i < count; ++i)
        v.PushBack(i);

    typedef izenelib::util::compressed_vector::OrderedVector::Iterator IteratorType;
    IteratorType iter(v);
    IteratorType end;
    i = 1;
    for(;iter !=end; ++iter)
    {
        BOOST_CHECK(*iter == (i ++));
    }
}

BOOST_AUTO_TEST_SUITE_END() // Compressed_vectortest

