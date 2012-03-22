/**
 * @file TestWorkerController.h
 * @brief a WorkerController used for test
 * @author Zhongxia Li, Jun Jiang
 * @date 2012-03-21
 */

#ifndef TEST_WORKER_CONTROLLER_H_
#define TEST_WORKER_CONTROLLER_H_

#include "worker_service.h"
#include <net/aggregator/WorkerController.h>

namespace net{
namespace aggregator{

class TestWorkerController : public WorkerController
{
public:
    TestWorkerController(boost::shared_ptr<SearchService> searchService)
    : searchService_(searchService)
    {
    }

    virtual bool addWorkerHandler(WorkerRouter& router)
    {
        ADD_WORKER_HANDLER_BEGIN(TestWorkerController)

        ADD_WORKER_HANDLER(getKeywordSearchResult)
        ADD_WORKER_HANDLER(add)

        ADD_WORKER_HANDLER_END()
    }

    WORKER_CONTROLLER_METHOD_2_OUT(getKeywordSearchResult, searchService_->getKeywordSearchResult, SearchRequest, SearchResult)

    WORKER_CONTROLLER_METHOD_2_OUT(add, processAdd, AddRequest, AddResult)

private:
    void processAdd(const AddRequest& req, AddResult& res)
    {
        res.i = req.i+req.j;
    }

private:
    boost::shared_ptr<SearchService> searchService_;
};

}} // end - namespace

#endif /* TEST_WORKER_CONTROLLER_H_ */
