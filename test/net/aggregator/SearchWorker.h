/**
 * @file SearchWorker.h
 * @author Zhongxia Li
 * @date Jun 27, 2011
 * @brief 
 */
#ifndef SEARCH_WORKER_H_
#define SEARCH_WORKER_H_

#include "TestParam.h"
#include <net/aggregator/BindCallProxyBase.h>

static const std::string TERM_ABC = "abc";
static const std::string TERM_CHINESE = "中文";
static const std::string ABC_DOC = "abc 123 ";
static const std::string CHINESE_DOC = "中文#@￥ **";

class SearchWorker : public net::aggregator::BindCallProxyBase<SearchWorker>
{
public:
    virtual bool bindCallProxy(CallProxyType& proxy)
    {
        BIND_CALL_PROXY_BEGIN(SearchWorker, proxy)
        BIND_CALL_PROXY_2(getKeywordSearchResult, SearchRequest, SearchResult)
        BIND_CALL_PROXY_2(add, AddRequest, AddResult)
        BIND_CALL_PROXY_END()
    }

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

    void add(const AddRequest& request, AddResult& result)
    {
        result.sum = request.i + request.j;
    }
};

#endif /* SEARCH_WORKER_H_ */
