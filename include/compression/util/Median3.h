/**
 * @file compression/util/Median3.h
 * @author Ian Yang
 * @date Created <2009-06-01 15:59:29>
 * @date Updated <2009-06-02 15:33:27>
 * @brief Finds median in 3 elements
 */
#ifndef COMPRESSION_UTIL_MEDIAN3_H
#define COMPRESSION_UTIL_MEDIAN3_H

namespace izenelib {
namespace compression {
namespace util {

/**
 * @brief Finds median in 3 elements, comparing using \c <.
 *
 * @param a an element
 * @param b an element
 * @param c an element
 *
 * @return returns \c j if i
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
    else if (c < b)
    {
        return b;
    }
    else if (c < a)
    {
        return c;
    }
    else
    {
        return a;
    }
}

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
    else if (less(c, b))
    {
        return b;
    }
    else if (less(c, a))
    {
        return c;
    }
    else
    {
        return a;
    }
}

}}} // namespace izenelib::compression::util

#endif // COMPRESSION_UTIL_MEDIAN3_H
