#ifndef SIMPLESERIALIZATION_H_
#define SIMPLESERIALIZATION_H_

#include <string>
#include <iostream>
#include <types.h>

namespace izenelib { namespace ir { namespace be_index {

template <typename T>
inline void serialize(const T & n, std::ostream & os)
{
    os.write((const char *)(&n), sizeof(n));
}

template <>
inline void serialize(const std::string & str, std::ostream & os)
{
    std::size_t size = str.size();
    os.write((const char *)(&size), sizeof(size));
    os.write(str.c_str(), size);
}

template <typename T>
inline void deserialize(std::istream & is, T & n)
{
    is.read((char *)(&n), sizeof(n));
}

template <>
inline void deserialize(std::istream & is, std::string & str)
{
    std::size_t size;
    is.read((char *)(&size), sizeof(size));
    str.resize(size);
    is.read(&(str[0]), size);
}

}}}

#endif
