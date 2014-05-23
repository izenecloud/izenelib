#include <util/CuckooFilter.h>
#include <util/Int2String.h>

#include <boost/test/unit_test.hpp>
#include <boost/timer.hpp>
#include <boost/thread.hpp>

#include <string>
#include <iostream>


BOOST_AUTO_TEST_CASE(CuckooFilter_test)
{
    const size_t bits_per_item = 12;
    const size_t total_items   = 1000000;

    // simple one:
    typedef izenelib::util::CuckooFilter<size_t, bits_per_item> CuckooFilterType;
    CuckooFilterType filter(total_items);
    // insert items
    size_t num_inserted = 0;
    for (size_t i = 0; i < total_items; i++, num_inserted++) {
        if (filter.Add(i) != CuckooFilterType::Ok) {
            break;
        }
    }

    // check existing items:
    // every item should be there
    for (size_t i = 0; i < num_inserted; i++) {
        BOOST_CHECK(filter.Contain(i) == CuckooFilterType::Ok);
    }

    // checking non-existing items
    // there are false positives
    size_t total_queries = 0;   
    size_t false_queries = 0;
    for (size_t i = total_items; i < 2 * total_items; i++) {
        if (filter.Contain(i) == CuckooFilterType::Ok) {
            false_queries++;
        }
        total_queries++;
    }

}
