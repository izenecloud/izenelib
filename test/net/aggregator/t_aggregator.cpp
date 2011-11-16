#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <net/aggregator/JobInfo.h>
#include <net/aggregator/JobAggregator.h>
#include <net/aggregator/AggregatorConfig.h>

#include "worker_service.h"
#include "worker_server.h"

#include <sstream>

using namespace boost;
using namespace boost::lambda;
using namespace net::aggregator;

typedef WorkerCaller<SearchService> LocalWorkerCaller;

class SearchAggregator : public JobAggregator<SearchAggregator, LocalWorkerCaller>
{
public:

    void initLocalWorkerCaller(boost::shared_ptr<SearchService> searchService)
    {
        localWorkerCaller_.reset(new LocalWorkerCaller);
        localWorkerCaller_->setInvoker(searchService.get());

        ADD_WORKER_CALLER_METHOD(LocalWorkerCaller, localWorkerCaller_, SearchService, getKeywordSearchResult);
    }

    bool aggregate(const std::string& func, SearchResult& res, const std::vector<std::pair<workerid_t, SearchResult> >& resList)
    {
        res.state = false;
        res.content = "";
        //std::stringstream ss;
        for (size_t i = 0; i < resList.size(); i++)
        {
            res.state |= resList[i].second.state;
            res.content += resList[i].second.content;
        }

        return res.state;
    }

    bool aggregate(const std::string& func, AddResult& res, const std::vector<std::pair<workerid_t, AddResult> >& resList)
    {
        res.i = 0;
        for (size_t i = 0; i < resList.size(); i++)
        {
           res.i += resList[i].second.i;
        }

        return true;
    }
};


class WorkersFixture
{
public:
    WorkersFixture()
    {
    }

    ~WorkersFixture()
    {
    }

    void startWorkers()
    {
        searchService1.reset(new SearchService());
        worker1.reset(new WorkerServer("localhost", 18111, searchService1));
        t1.reset( new thread(lambda::bind(&WorkerServer::start, var(*worker1))) );

        searchService2.reset(new SearchService());
        worker2.reset(new WorkerServer("localhost", 18112, searchService2));
        t2.reset( new thread(lambda::bind(&WorkerServer::start, var(*worker2))) );

        sleep(1);
    }

    void stopWorkers()
    {
        worker1->end();
        worker1->join();
        worker2->end();
        worker2->join();
        t1->join();
        t2->join();
    }

    boost::shared_ptr<SearchService> searchService1;
    boost::shared_ptr<WorkerServer> worker1;
    boost::shared_ptr<thread> t1;

    boost::shared_ptr<SearchService> searchService2;
    boost::shared_ptr<WorkerServer> worker2;
    boost::shared_ptr<thread> t2;
};

BOOST_FIXTURE_TEST_SUITE( t_aggregator, WorkersFixture )

BOOST_AUTO_TEST_CASE( aggregator_remote_workers )
{
    std::cout << "--- Aggregate data from remote workers" << std::endl;

    startWorkers();

    // aggregator
    AggregatorConfig config;
    config.addWorker("0.0.0.0", 18111, 1);
    config.addWorker("0.0.0.0", 18112, 2);

    SearchAggregator ag;
    ag.debug_ = true;
    ag.setAggregatorConfig(config);

    SearchRequest req;
    req.keyword = TERM_ABC;
    SearchResult res;
    ag.distributeRequest("", "getKeywordSearchResult", req, res);

    BOOST_CHECK_EQUAL(res.state, true);
    BOOST_CHECK_EQUAL(res.content, (ABC_DOC+ABC_DOC));

    stopWorkers();
}

BOOST_AUTO_TEST_CASE( aggregator_local_remote_workers )
{
    std::cout << "--- Aggregate data from local and remote workers." << std::endl;

    startWorkers();

    boost::shared_ptr<SearchService> localWorkerService(new SearchService);

    // aggregator
    AggregatorConfig config;
    config.addWorker("0.0.0.0", 18110, 1, true); // is local
    config.addWorker("0.0.0.0", 18111, 2);

    SearchAggregator ag;
    ag.debug_ = true;
    ag.setAggregatorConfig(config);
    ag.initLocalWorkerCaller(localWorkerService);

    SearchRequest req;
    req.keyword = TERM_ABC;
    SearchResult res;
    ag.distributeRequest("", "getKeywordSearchResult", req, res);

    BOOST_CHECK_EQUAL(res.state, true);
    BOOST_CHECK_EQUAL(res.content, (ABC_DOC+ABC_DOC));

    stopWorkers();
}

BOOST_AUTO_TEST_CASE( aggregator_group_requests )
{
    std::cout << "--- Aggregator send a group of requests." << std::endl;

    startWorkers();

    boost::shared_ptr<SearchService> localWorkerService(new SearchService);

    // aggregator
    AggregatorConfig config;
    config.addWorker("0.0.0.0", 18110, 1, true); // is local
    config.addWorker("0.0.0.0", 18111, 2);
    config.addWorker("0.0.0.0", 18112, 3);

    SearchAggregator aggregator;
    aggregator.debug_ = true;
    aggregator.setAggregatorConfig(config);
    aggregator.initLocalWorkerCaller(localWorkerService);

    SearchRequest req0;
    req0.keyword = TERM_ABC;
    SearchRequest req1;
    req1.keyword = TERM_CHINESE;
    SearchRequest req2;
    req2.keyword = TERM_ABC;

    SearchResult res;

    RequestGroup<SearchRequest, SearchResult> requestGroup;
    requestGroup.addRequest(1, &req0, &res);
    requestGroup.addRequest(2, &req1, &res);
    requestGroup.addRequest(3, &req2, &res);

    aggregator.distributeRequest("", "getKeywordSearchResult", requestGroup, res);

    BOOST_CHECK_EQUAL(res.state, true);
    BOOST_CHECK_EQUAL(res.content, ABC_DOC+CHINESE_DOC+ABC_DOC);

    stopWorkers();
}

BOOST_AUTO_TEST_SUITE_END()
