#ifndef UTIL_FUNCTIONAL_H
#define UTIL_FUNCTIONAL_H
/**
 * @file util/functional.h
 * @author Ian Yang
 * @date Created <2009-10-12 12:20:03>
 * @date Updated <2009-10-12 12:28:55>
 * @brief helper functions for std algorithms
 */
#include <functional>

namespace izenelib {
namespace util {

template<typename T>
struct first_equal_to : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return x.first == y.first;
    }
};

template<typename T>
struct first_not_equal_to : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return !(x.first == y.first);
    }
};

template<typename T>
struct first_less : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return x.first < y.first;
    }
};

template<typename T>
struct first_less_equal : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return x.first < y.first || x.first == y.first;
    }
};

template<typename T>
struct first_greater : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return y.first < x.first;
    }
};

template<typename T>
struct first_greater_equal : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return y.first < x.first || y.first == x.first;
    }
};

template<typename T>
struct second_equal_to : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return x.second == y.second;
    }
};

template<typename T>
struct second_not_equal_to : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return !(x.second == y.second);
    }
};

template<typename T>
struct second_less : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return x.second < y.second;
    }
};

template<typename T>
struct second_less_equal : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return x.second < y.second || x.second == y.second;
    }
};

template<typename T>
struct second_greater : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return y.second < x.second;
    }
};

template<typename T>
struct second_greater_equal : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
        return y.second < x.second || y.second == x.second;
    }
};

}} // namespace izenelib::util

#endif // UTIL_FUNCTIONAL_H
