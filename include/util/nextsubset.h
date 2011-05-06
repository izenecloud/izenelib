#ifndef IZENELIB_UTIL_NEXT_SUBSET_H
#define IZENELIB_UTIL_NEXT_SUBSET_H

#include <algorithm>

namespace izenelib{ namespace util{

template< class I, class O > // I forward, O bidirectional iterator
O next_subset( I uni_first, I uni_last, // set universe in a range
    O sub_first, O sub_last ) { // current subset in a range
    std::pair< O, I > mis = std::mismatch( sub_first, sub_last, uni_first );
    if ( mis.second == uni_last ) return sub_first; // finished cycle

    O ret;
    if ( mis.first == sub_first ) { // copy elements following mismatch
        std::copy_backward( mis.first, sub_last, ++ (ret = sub_last) ); 
    } else ret = std::copy( mis.first, sub_last, ++ O(sub_first) ); 
    * sub_first = * mis.second; // add first element not yet in result
    return ret; // return end of new subset. (Output range must accommodate.)
}

}}

#endif

