/**
 * @file compression/util/Median3.h
 * @author Ian Yang
 * @date Created <2009-06-01 15:59:29>
 * @date Updated <2009-06-03 17:00:44>
 * @brief Finds median in 3 elements
 */
#ifndef COMPRESSION_UTIL_MEDIAN3_H
#define COMPRESSION_UTIL_MEDIAN3_H

namespace izenelib {
namespace util{
namespace compression {

/**
 * @brief Finds median in 3 elements, comparing using \c <.
 *
 * @param a an element
 * @param b an element
 * @param c an element
 *
 * @return one of \a a, \a b or \a c.
 *
 * if \c {l,m,n} is a permutation of \a (a,b,c) such that \c l<=m<=n then the
 * value returned with be \c m.
 */
template<typename T>
const T& median3(const T& a, const T& b, const T& c)
{
    if (a < b)
    {
        if (b < c)
        {
            return b;
        }
        else if (a < c)
        {
            return c;
        }
        else
        {
            return a;
        }
    }
    else if (a < c)
    {
        return a;
    }
    else if (b < c)
    {
        return c;
    }
    else
    {
        return b;
    }
}

/**
 * @brief Finds median in 3 elements, comparing using a predicate.
 *
 * @param a an element
 * @param b an element
 * @param c an element
 * @param less a binary predicate.
 *
 * @return one of \a a, \a b or \a c.
 *
 * if \c {l,m,n} is a permutation of \a (a,b,c) such that \c less(m,l) and \c
 * less(n,m) are both \c false then the value returned with be \c m.
 */
template<typename T, typename CompareT>
const T& median3(const T& a, const T& b, const T& c, CompareT less)
{
    if (less(a, b))
    {
        if (less(b, c))
        {
            return b;
        }
        else if (less(a, c))
        {
            return c;
        }
        else
        {
            return a;
        }
    }
    else if (less(a, c))
    {
        return a;
    }
    else if (less(b, c))
    {
        return c;
    }
    else
    {
        return b;
    }
}

}}} // namespace izenelib::util::compression

#endif // COMPRESSION_UTIL_MEDIAN3_H
