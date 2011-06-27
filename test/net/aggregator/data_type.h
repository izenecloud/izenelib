/**
 * @file data_type.h
 * @author Zhongxia Li
 * @date Jun 27, 2011
 * @brief 
 */
#ifndef DATA_TYPE_H_
#define DATA_TYPE_H_

#include <3rdparty/msgpack/msgpack.hpp>

struct Data
{
    int i;
    std::string s;

    Data()
    : i(0), s()
    {}

    MSGPACK_DEFINE(s, i);
};

struct DataResult
{
    int i;
    std::string s;
    std::string err;

    DataResult()
    : i(), s(), err()
    {}

    MSGPACK_DEFINE(i, s, err);
};


#endif /* DATA_TYPE_H_ */
