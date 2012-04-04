/**
 * @file SearchMerger.h
 * @author Zhongxia Li
 * @date Jun 27, 2011
 * @brief 
 */
#ifndef SEARCH_MERGER_H_
#define SEARCH_MERGER_H_

#include "TestParam.h"
#include <net/aggregator/BindCallProxyBase.h>
#include <net/aggregator/WorkerResults.h>

class SearchMerger : public net::aggregator::BindCallProxyBase<SearchMerger>
{
public:
    virtual bool bindCallProxy(CallProxyType& proxy)
    {
        BIND_CALL_PROXY_BEGIN(SearchMerger, proxy)
        BIND_CALL_PROXY_2(getKeywordSearchResult, net::aggregator::WorkerResults<SearchResult>, SearchResult)
        BIND_CALL_PROXY_2(add, net::aggregator::WorkerResults<AddResult>, AddResult)
        BIND_CALL_PROXY_END()
    }

    void getKeywordSearchResult(const net::aggregator::WorkerResults<SearchResult>& workerResults, SearchResult& mergeResult)
    {
        mergeResult.state = false;
        mergeResult.content = "";
        for (size_t i = 0; i < workerResults.size(); i++)
        {
            mergeResult.state |= workerResults.result(i).state;
            mergeResult.content += workerResults.result(i).content;
        }
    }

    void add(const net::aggregator::WorkerResults<AddResult>& workerResults, AddResult& mergeResult)
    {
        mergeResult.sum = 0;
        for (size_t i = 0; i < workerResults.size(); i++)
        {
           mergeResult.sum += workerResults.result(i).sum;
        }
    }
};

#endif /* SEARCH_MERGER_H_ */
