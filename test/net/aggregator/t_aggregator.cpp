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


class SearchAggregator : public JobAggregator<SearchAggregator, LocalWorkerCaller>
{
public:

    void initLocalWorkerCaller(boost::shared_ptr<SearchService> searchService)
    {
        localWorkerCaller_.reset(new LocalWorkerCaller);
        localWorkerCaller_->setInvoker(searchService.get());

        LocalWorkerCaller::func_t func = (LocalWorkerCaller::func_t) &SearchService::getKeywordSearchResult;
        localWorkerCaller_->addMethod("getKeywordSearchResult", func);
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


BOOST_AUTO_TEST_SUITE( t_aggregator )

BOOST_AUTO_TEST_CASE( start_workers )
{
    std::cout << "Before test, worker servers have to be started firstly :" << std::endl;
    std::cout << " testbin$ ./t_worker localhost 18111 " << std::endl;
    std::cout << " testbin$ ./t_worker localhost 18112 " << std::endl;

//    std::string host = "0.0.0.0";
//    uint16_t port = 18111;
//    boost::shared_ptr<SearchService> searchService(new SearchService());
//    WorkerServer worker(host, port, searchService);
//    thread t(lambda::bind(&WorkerServer::start, var(worker)));
//
//    t.join();
}

BOOST_AUTO_TEST_CASE( aggregator_remote_workers )
{
    std::cout << "--- Aggregate data from remote workers" << std::endl;

    AggregatorConfig config;
    config.enableLocalWorker_ = false; // disable local
    config.addWorker("0.0.0.0", 18111);
    config.addWorker("0.0.0.0", 18112);

    SearchAggregator ag;
    ag.debug_ = true;
    ag.setAggregatorConfig(config);

    SearchRequest req;
    req.keyword = TERM_ABC;
    SearchResult res;
    ag.distributeRequest("", "getKeywordSearchResult", req, res);

    BOOST_CHECK_EQUAL(res.state, true);
    BOOST_CHECK_EQUAL(res.content, (ABC_DOC+ABC_DOC));
}

BOOST_AUTO_TEST_CASE( aggregator_local_remote_workers )
{
    std::cout << "--- Aggregate data from local and remote workers." << std::endl;

    boost::shared_ptr<SearchService> workerService(new SearchService);

    AggregatorConfig config;
    config.enableLocalWorker_ = true;
    config.addWorker("0.0.0.0", 18111);

    SearchAggregator ag;
    ag.debug_ = true;
    ag.setAggregatorConfig(config);
    ag.initLocalWorkerCaller(workerService);

    SearchRequest req;
    req.keyword = TERM_ABC;
    SearchResult res;
    ag.distributeRequest("", "getKeywordSearchResult", req, res);

    BOOST_CHECK_EQUAL(res.state, true);
    BOOST_CHECK_EQUAL(res.content, (ABC_DOC+ABC_DOC));
}

BOOST_AUTO_TEST_CASE( aggregator_group_requests )
{
    std::cout << "--- Aggregator send a group of requests." << std::endl;

    boost::shared_ptr<SearchService> workerService(new SearchService);

    AggregatorConfig config;
    config.enableLocalWorker_ = true;
    config.addWorker("0.0.0.0", 18111);
    config.addWorker("0.0.0.0", 18112);

    SearchAggregator aggregator;
    aggregator.debug_ = true;
    aggregator.setAggregatorConfig(config);
    aggregator.initLocalWorkerCaller(workerService);

    SearchRequest req0;
    req0.keyword = TERM_ABC;
    SearchRequest req1;
    req1.keyword = TERM_CHINESE;
    SearchRequest req2;
    req2.keyword = TERM_ABC;

    SearchResult res;

    RequestGroup<SearchRequest, SearchResult> requestGroup;
    requestGroup.addRequest(0, &req0, &res);
    requestGroup.addRequest(1, &req1, &res);
    requestGroup.addRequest(2, &req2, &res);

    aggregator.distributeRequest("", "getKeywordSearchResult", requestGroup, res);

    BOOST_CHECK_EQUAL(res.state, true);
    BOOST_CHECK_EQUAL(res.content, ABC_DOC+CHINESE_DOC+ABC_DOC);
}

BOOST_AUTO_TEST_SUITE_END()
