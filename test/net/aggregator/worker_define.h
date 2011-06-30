/**
 * @file worker_define.h
 * @author Zhongxia Li
 * @date Jun 30, 2011
 * @brief 
 */
#ifndef WORKER_DEFINE_H_
#define WORKER_DEFINE_H_

#include <net/aggregator/JobInfo.h>
#include <net/aggregator/JobWorker.h>
#include <net/aggregator/WorkerHandler.h>

using namespace net::aggregator;

class SearchWorker : public JobWorker<SearchWorker>
{

public:
    SearchWorker(const std::string& host, uint16_t port, boost::shared_ptr<SearchService> searchService)
    :JobWorker<SearchWorker>(host, port)
    ,searchService_(searchService)
    {
    }

public:
    /*pure virtual*/
    void addHandlers()
    {
        ADD_WORKER_HANDLER_LIST_BEGIN( SearchWorker )

        ADD_WORKER_HANDLER( getKeywordSearchResult )
        ADD_WORKER_HANDLER( add )

        ADD_WORKER_HANDLER_LIST_END()
    }

    bool getKeywordSearchResult(JobRequest& req)
    {
        WORKER_HANDLE_1_1(req, Data, processGetKeywordSearchResult, DataResult)
        return true;
    }

    bool add(JobRequest& req)
    {
        WORKER_HANDLE_1_1(req, AddData, processAdd, int)
        return true;
    }

public:
    /// call as local worker
    bool call(
            const std::string& func,
            const Data& request,
            DataResult& result,
            std::string& error)
    {
        if (func == "getKeywordSearchResult")
        {
            return processGetKeywordSearchResult(request, result);
        }

        error = "no method!";

        return false;
    }

    bool call(
            const std::string& func,
            const AddData& request,
            int& result,
            std::string& error)
    {
        if (func == "add")
        {
            return processAdd(request, result);
        }

        error = "no method!";

        return false;
    }

private:

    bool processGetKeywordSearchResult(const Data& param, DataResult& result)
    {
        searchService_->getKeywordSearchResult(param.s, result.s);
        //sleep(5);
        return true;
    }

    bool processAdd(const AddData& param, int& result)
    {
        result = param.i + param.j;
        return true;
    }

private:
    boost::shared_ptr<SearchService> searchService_;
};

#endif /* WORKER_DEFINE_H_ */
