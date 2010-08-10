/**
 * @file compression/util/PseudoMedian.h
 * @author Ian Yang
 * @date Created <2009-06-02 11:23:55>
 * @date Updated <2009-06-03 17:13:27>
 */
#ifndef COMPRESSION_UTIL_PSEUDO_MEDIAN_H
#define COMPRESSION_UTIL_PSEUDO_MEDIAN_H

#include <iterator>
#include <stdexcept>

#include <boost/next_prior.hpp>

#include <compression/util/Median3.h>

namespace izenelib {
namespace compression {
namespace util {

/**
 * @brief finds approximate median in a range, algorithm by Bentley & McIlroy.
 *
 * @tparam I Bidirectional Iterator
 * @param first range start
 * @param last range end
 * @return approximate median value
 * @pre \c [first,last) is valid non-empty range
 *
 * - Returns middle element as median for small array.
 * - Returns median of 3 elements, first, last and middle, for middle size array.
 * - Find out median from 9 elements for large scale array.
 */
template<typename I>
const typename std::iterator_traits<I>::value_type&
pseudoMedian(I first, I last)
{
    BOOST_ASSERT(first != last);
    if (first == last)
    {
        throw std::out_of_range("range is empty in - izenelib::compression::pseudoMedian");
    }

    typedef typename std::iterator_traits<I>::difference_type difference_type;
    typedef const typename std::iterator_traits<I>::value_type& reference;

    difference_type size = std::distance(first, last);
    I middle = boost::next(first, size >> 1);

    if (size > 40)
    {
        difference_type step = size >> 3;
        return median3(
            median3(
                *first,
                *boost::next(first, step),
                *boost::next(first, 2 * step)
            ),
            median3(
                *boost::prior(middle, step),
                *middle,
                *boost::next(middle, step)
            ),
            median3(
                *boost::prior(last),
                *boost::prior(last, step + 1),
                *boost::prior(last, 2 * step + 1)
            )
        );
    }
    else if (size > 7)
    {
        return median3(*first, *middle, *boost::prior(last));
    }
    else
    {
        return *middle;
    }
}

/**
 * @brief finds approximate median value in a range, algorithm by Bentley &
 * McIlroy. Compares values using a predicate
 *
 * @tparam I Bidirectional Iterator
 * @param first range start
 * @param last range end
 * @param less a predicate
 * @return the approximate median value
 * @pre \c [first,last) is valid non-empty range
 *
 * - Returns middle element as median for small array.
 * - Returns median of 3 elements, first, last and middle, for middle size array.
 * - Find out median from 9 elements for large scale array.
 *
 * Use \a less to compare. Consider \c a<b if \c less(a,b) returns \c true.
 */
template<typename I, typename CompareT>
const typename std::iterator_traits<I>::value_type&
pseudoMedian(I first, I last, CompareT less)
{
    BOOST_ASSERT(first != last);
    if (first == last)
    {
        throw std::out_of_range("range is empty in - izenelib::compression::pseudoMedian");
    }

    typedef typename std::iterator_traits<I>::difference_type difference_type;
    typedef const typename std::iterator_traits<I>::value_type& reference;

    difference_type size = std::distance(first, last);
    I middle = boost::next(first, size >> 1);

    if (size > 40)
    {
        difference_type step = size >> 3;
        return median3(
            median3(
                *first,
                *boost::next(first, step),
                *boost::next(first, 2 * step),
                less
            ),
            median3(
                *boost::prior(middle, step),
                *middle,
                *boost::next(middle, step),
                less
            ),
            median3(
                *boost::prior(last),
                *boost::prior(last, step + 1),
                *boost::prior(last, 2 * step + 1),
                less
            )
        );
    }
    else if (size > 7)
    {
        return median3(*first, *middle, *boost::prior(last), less);
    }
    else
    {
        return *middle;
    }
}

}}} // namespace izenelib::compression::util

#endif // COMPRESSION_UTIL_PSEUDO_MEDIAN_H
