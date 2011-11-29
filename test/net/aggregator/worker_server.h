/**
 * @file worker_server.h
 * @author Zhongxia Li
 * @date Jun 30, 2011
 * @brief 
 */
#ifndef WORKER_SERVER_H_
#define WORKER_SERVER_H_

#include <net/aggregator/Typedef.h>
#include <net/aggregator/WorkerServerBase.h>
#include <net/aggregator/WorkerServerHandler.h>

#include "worker_service.h"

using namespace net::aggregator;


class WorkerServer : public WorkerServerBase<WorkerServer>
{

public:
    WorkerServer(const std::string& host, uint16_t port, boost::shared_ptr<SearchService> searchService)
    :WorkerServerBase<WorkerServer>(host, port)
    ,searchService_(searchService)
    {
    }

public:
    /*pure virtual*/
    void addServerHandlers()
    {
        ADD_WORKER_HANDLER_LIST_BEGIN( WorkerServer )

        ADD_WORKER_HANDLER( getKeywordSearchResult )
        ADD_WORKER_HANDLER( add )

        ADD_WORKER_HANDLER_LIST_END()
    }

    void  getKeywordSearchResult(request_t& req)
    {
        WORKER_HANDLE_1_1(req, SearchRequest, searchService_->getKeywordSearchResult, SearchResult)
    }

    void add(request_t& req)
    {
        WORKER_HANDLE_1_1(req, AddRequest, processAdd, AddResult)
    }

private:
    void processAdd(const AddRequest& req, AddResult& res)
    {
        res.i = req.i+req.j;
    }

private:
    boost::shared_ptr<SearchService> searchService_;
};

#endif /* WORKER_SERVER_H_ */
