/**
 * @file TestWorkerController.h
 * @brief a WorkerController used for test
 * @author Zhongxia Li, Jun Jiang
 * @date 2012-03-21
 */

#ifndef TEST_WORKER_CONTROLLER_H_
#define TEST_WORKER_CONTROLLER_H_

#include "SearchWorker.h"
#include <net/aggregator/WorkerController.h>

namespace net{
namespace aggregator{

class TestWorkerController : public WorkerController
{
public:
    TestWorkerController(boost::shared_ptr<SearchWorker> searchWorker)
    : searchWorker_(searchWorker)
    {
    }

    virtual bool addWorkerHandler(WorkerRouter& router)
    {
        ADD_WORKER_HANDLER_BEGIN(TestWorkerController, router)

        ADD_WORKER_HANDLER(getKeywordSearchResult)
        ADD_WORKER_HANDLER(add)

        ADD_WORKER_HANDLER_END()
    }

    WORKER_CONTROLLER_METHOD_2(getKeywordSearchResult, searchWorker_->getKeywordSearchResult, SearchRequest, SearchResult)

    WORKER_CONTROLLER_METHOD_2(add, searchWorker_->add, AddRequest, AddResult)

private:
    boost::shared_ptr<SearchWorker> searchWorker_;
};

}} // end - namespace

#endif /* TEST_WORKER_CONTROLLER_H_ */
