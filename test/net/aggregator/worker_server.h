/**
 * @file worker_server.h
 * @author Zhongxia Li
 * @date Jun 30, 2011
 * @brief 
 */
#ifndef WORKER_SERVER_H_
#define WORKER_SERVER_H_

#include <net/aggregator/JobInfo.h>
#include <net/aggregator/JobWorker.h>
#include <net/aggregator/WorkerHandler.h>

#include "worker_service.h"

using namespace net::aggregator;

typedef WorkerCaller<SearchService> LocalWorkerCaller;

class WorkerServer : public JobWorker<WorkerServer>
{

public:
    WorkerServer(const std::string& host, uint16_t port, boost::shared_ptr<SearchService> searchService)
    :JobWorker<WorkerServer>(host, port)
    ,searchService_(searchService)
    {
    }

public:
    /*pure virtual*/
    void addHandlers()
    {
        ADD_WORKER_HANDLER_LIST_BEGIN( WorkerServer )

        ADD_WORKER_HANDLER( getKeywordSearchResult )
        ADD_WORKER_HANDLER( add )

        ADD_WORKER_HANDLER_LIST_END()
    }

    bool getKeywordSearchResult(JobRequest& req)
    {
        WORKER_HANDLE_REQUEST_1_1(req, SearchRequest, SearchResult, searchService_, getKeywordSearchResult)
        return true;
    }

    bool add(JobRequest& req)
    {
        WORKER_HANDLE_1_1(req, AddRequest, processAdd, AddResult)
        return true;
    }

private:
    void processAdd(const AddRequest& req, AddResult& res)
    {
        res.i = req.i+req.j;
    }

private:
    boost::shared_ptr<SearchService> searchService_;
};

//class LocalWorkerCaller : public SearchService
//{
//public:
//    LocalWorkerCaller()
//    {
//    	funcMap_["getKeywordSearchResult"] = (func_t) &LocalWorkerCaller::getKeywordSearchResult;
//
//    }
//
//    template <typename RequestType, typename ResultType>
//    bool call(
//            const std::string& func,
//            const RequestType& request,
//            ResultType& result,
//            std::string& error)
//    {
//    	typedef void(LocalWorkerCaller::*real_func_t)(const RequestType& , ResultType&);
//
//        cout << "LocalWorkerCaller::call "<<func<<endl;
//        if (func == "getKeywordSearchResult")
//        {
//        	real_func_t f = (real_func_t) funcMap_["getKeywordSearchResult"];
//        	(this->*f)(request, result);
//        }
//        return true;
//    }
//
//    typedef void(LocalWorkerCaller::*func_t)(void);
//
//    std::map<std::string, func_t> funcMap_;
//};

#endif /* WORKER_SERVER_H_ */
