#ifndef CONCURRENT_HASH_H
#define CONCURRENT_HASH_H

#include <stdint.h>
#include <cstddef>
#include <limits>
#include <boost/functional/hash.hpp>
#include <util/ustring/UString.h>

namespace concurrent{

/// replace with boost::hash
template<typename T>
inline std::size_t hash_value(const T& key)
{
    static boost::hash<T> hasher;
    return hasher(key);
}

template<>
inline std::size_t hash_value<izenelib::util::UString>(const izenelib::util::UString& key)
{
    static boost::hash<std::string> hasher;
    std::string str;
    key.convertString(str, izenelib::util::UString::UTF_8);
    return hasher(str);
}

// std::size_t hash_value(bool);
// std::size_t hash_value(char);
// std::size_t hash_value(unsigned char);
// std::size_t hash_value(signed char);
// std::size_t hash_value(short);
// std::size_t hash_value(unsigned short);
// std::size_t hash_value(int);
// std::size_t hash_value(unsigned int);
// std::size_t hash_value(long);
// std::size_t hash_value(unsigned long);
// 
// 
// template <class T> std::size_t hash_value(T*);
// 
// std::size_t hash_value(float v);
// std::size_t hash_value(double v);
// std::size_t hash_value(long double v);
// 
// 
// template <class T>
// inline std::size_t hash_value_signed(T val)
// {
//     const int size_t_bits = std::numeric_limits<std::size_t>::digits;
//     const int length = (std::numeric_limits<T>::digits - 1)
//                        / size_t_bits;
// 
//     std::size_t seed = 0;
//     T positive = val < 0 ? -1 - val : val;
// 
//     for (unsigned int i = length * size_t_bits; i > 0; i -= size_t_bits)
//     {
//         seed ^= (std::size_t) (positive >> i) + (seed<<6) + (seed>>2);
//     }
//     seed ^= (std::size_t) val + (seed<<6) + (seed>>2);
// 
//     return seed;
// }
// 
// template <class T>
// inline std::size_t hash_value_unsigned(T val)
// {
//     const int size_t_bits = std::numeric_limits<std::size_t>::digits;
// // ceiling(std::numeric_limits<T>::digits / size_t_bits) - 1
//     const int length = (std::numeric_limits<T>::digits - 1)
//                        / size_t_bits;
// 
//     std::size_t seed = 0;
// 
// // Hopefully, this loop can be unrolled.
//     for (unsigned int i = length * size_t_bits; i > 0; i -= size_t_bits)
//     {
//         seed ^= (std::size_t) (val >> i) + (seed<<6) + (seed>>2);
//     }
//     seed ^= (std::size_t) val + (seed<<6) + (seed>>2);
// 
//     return seed;
// }
// 
// 
// inline std::size_t hash_value(bool v)
// {
//     return static_cast<std::size_t>(v);
// }
// 
// inline std::size_t hash_value(char v)
// {
//     return static_cast<std::size_t>(v);
// }
// 
// inline std::size_t hash_value(unsigned char v)
// {
//     return static_cast<std::size_t>(v);
// }
// 
// inline std::size_t hash_value(signed char v)
// {
//     return static_cast<std::size_t>(v);
// }
// 
// inline std::size_t hash_value(short v)
// {
//     return static_cast<std::size_t>(v);
// }
// 
// inline std::size_t hash_value(unsigned short v)
// {
//     return static_cast<std::size_t>(v);
// }
// 
// inline std::size_t hash_value(int v)
// {
//     return static_cast<std::size_t>(v);
// }
// 
// inline std::size_t hash_value(unsigned int v)
// {
//     return static_cast<std::size_t>(v);
// }
// 
// inline std::size_t hash_value(long v)
// {
//     return static_cast<std::size_t>(v);
// }
// 
// inline std::size_t hash_value(unsigned long v)
// {
//     return static_cast<std::size_t>(v);
// }

}
#endif /* CONCURRENT_HASH_H */
