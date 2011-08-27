/**
 * @file worker_service.h
 * @author Zhongxia Li
 * @date Jun 27, 2011
 * @brief 
 */
#ifndef WORKER_SERVICE_H_
#define WORKER_SERVICE_H_

#include <3rdparty/msgpack/msgpack.hpp>

struct AddRequest
{
    int i;
    int j;

    MSGPACK_DEFINE(i, j);
};

struct AddResult
{
    int i;
    std::string s;

    AddResult()
    : i(0), s()
    {}

    MSGPACK_DEFINE(s, i);
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

static const std::string TERM_ABC = "abc";
static const std::string TERM_CHINESE = "中文";
static const std::string ABC_DOC = "abc 123 ";
static const std::string CHINESE_DOC = "中文#@￥ **";

class SearchService
{
public:
    SearchService()
    {
    }

public:
    void getKeywordSearchResult(const SearchRequest& request, SearchResult& result)
    {
        if (request.keyword == TERM_ABC)
        {
            result.content = ABC_DOC;
            result.state = true;
        }
        else if (request.keyword == TERM_CHINESE)
        {
            result.content = CHINESE_DOC;
            result.state = true;
        }
        else
        {
            result.content = "null ";
            result.state = false;
        }
    }

private:

};


#endif /* WORKER_SERVICE_H_ */
