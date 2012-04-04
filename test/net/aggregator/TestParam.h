/**
 * @file TestParam.h
 * @author Zhongxia Li
 * @date Jun 27, 2011
 * @brief 
 */
#ifndef TEST_PARAM_H_
#define TEST_PARAM_H_

#include <3rdparty/msgpack/msgpack.hpp>

struct AddRequest
{
    int i;
    int j;

    AddRequest(int a = 0, int b = 0)
    : i(a), j(b)
    {}

    MSGPACK_DEFINE(i, j);
};

struct AddResult
{
    int sum;
    std::string s;

    AddResult()
    : sum(0), s()
    {}

    MSGPACK_DEFINE(s, sum);
};

struct SearchRequest
{
    std::string keyword;
    MSGPACK_DEFINE(keyword);
};

struct SearchResult
{
    bool state;
    std::string content;
    std::string err;

    SearchResult()
    : state(false), content(), err()
    {}

    MSGPACK_DEFINE(state, content, err);
};

#endif /* TEST_PARAM_H_ */
