#include "SearchWorker.h"
#include "SearchMerger.h"
#include "TestWorkerController.h"

#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>

#include <net/aggregator/Aggregator.h>
#include <net/aggregator/CallProxy.h>
#include <net/aggregator/AggregatorConfig.h>
#include <net/aggregator/Typedef.h>
#include <net/aggregator/WorkerServer.h>

using namespace boost;
using namespace net::aggregator;

typedef CallProxy<SearchMerger> SearchMergerProxy;
typedef CallProxy<SearchWorker> SearchWorkerProxy;
typedef Aggregator<SearchMergerProxy, SearchWorkerProxy> SearchAggregator;

class WorkersFixture
{
public:
    WorkersFixture()
    : worker1(new SearchWorker)
    , worker2(new SearchWorker)
    {
        BOOST_CHECK(TestWorkerController(worker1).addWorkerHandler(router1));
        server1.reset(new WorkerServer(router1, "localhost", 18111));

        BOOST_CHECK(TestWorkerController(worker2).addWorkerHandler(router2));
        server2.reset(new WorkerServer(router2, "localhost", 18112));

        server1->start();
        server2->start();
    }

    WorkerRouter router1;
    WorkerRouter router2;

    boost::shared_ptr<SearchWorker> worker1;
    boost::shared_ptr<WorkerServer> server1;

    boost::shared_ptr<SearchWorker> worker2;
    boost::shared_ptr<WorkerServer> server2;
};

BOOST_FIXTURE_TEST_SUITE( t_aggregator, WorkersFixture )

BOOST_AUTO_TEST_CASE( aggregator_remote_workers )
{
    std::cout << "--- Aggregate data from remote workers" << std::endl;

    // aggregator
    AggregatorConfig config;
    config.addWorker("0.0.0.0", 18111, 1);
    config.addWorker("0.0.0.0", 18112, 2);

    boost::scoped_ptr<SearchMerger> searchMerger(new SearchMerger);
    SearchMergerProxy* mergerProxy = new SearchMergerProxy(searchMerger.get());
    BOOST_CHECK(searchMerger->bindCallProxy(*mergerProxy));

    SearchAggregator ag(mergerProxy);
    ag.setDebug(true);
    ag.setAggregatorConfig(config);

    {
    SearchRequest req;
    req.keyword = TERM_ABC;
    SearchResult res;
    BOOST_CHECK(ag.distributeRequest("", "getKeywordSearchResult", req, res));

    BOOST_CHECK_EQUAL(res.state, true);
    BOOST_CHECK_EQUAL(res.content, (ABC_DOC+ABC_DOC));
    }

    {
    AddRequest addRequest(1, 2);
    AddResult addResult;
    BOOST_CHECK(ag.distributeRequest("", "add", addRequest, addResult));
    BOOST_CHECK_EQUAL(addResult.sum, (1+2)*2);
    }
}

BOOST_AUTO_TEST_CASE( aggregator_local_remote_workers )
{
    std::cout << "--- Aggregate data from local and remote workers." << std::endl;

    boost::scoped_ptr<SearchMerger> searchMerger(new SearchMerger);
    SearchMergerProxy* mergerProxy = new SearchMergerProxy(searchMerger.get());
    BOOST_CHECK(searchMerger->bindCallProxy(*mergerProxy));

    boost::scoped_ptr<SearchWorker> localWorker(new SearchWorker);
    SearchWorkerProxy* workerProxy = new SearchWorkerProxy(localWorker.get());
    BOOST_CHECK(localWorker->bindCallProxy(*workerProxy));

    // aggregator
    AggregatorConfig config;
    config.addWorker("0.0.0.0", 18110, 1, true); // is local
    config.addWorker("0.0.0.0", 18111, 2);

    SearchAggregator ag(mergerProxy, workerProxy);
    ag.setDebug(true);
    ag.setAggregatorConfig(config);

    {
    SearchRequest req;
    req.keyword = TERM_ABC;
    SearchResult res;
    BOOST_CHECK(ag.distributeRequest("", "getKeywordSearchResult", req, res));

    BOOST_CHECK_EQUAL(res.state, true);
    BOOST_CHECK_EQUAL(res.content, (ABC_DOC+ABC_DOC));
    }

    {
    AddRequest addRequest(1, 2);
    AddResult addResult;
    BOOST_CHECK(ag.distributeRequest("", "add", addRequest, addResult));
    BOOST_CHECK_EQUAL(addResult.sum, (1+2)*2);
    }
}

BOOST_AUTO_TEST_CASE( aggregator_group_requests )
{
    std::cout << "--- Aggregator send a group of requests." << std::endl;

    boost::scoped_ptr<SearchMerger> searchMerger(new SearchMerger);
    SearchMergerProxy* mergerProxy = new SearchMergerProxy(searchMerger.get());
    BOOST_CHECK(searchMerger->bindCallProxy(*mergerProxy));

    boost::scoped_ptr<SearchWorker> localWorker(new SearchWorker);
    SearchWorkerProxy* workerProxy = new SearchWorkerProxy(localWorker.get());
    BOOST_CHECK(localWorker->bindCallProxy(*workerProxy));

    // aggregator
    AggregatorConfig config;
    config.addWorker("0.0.0.0", 18110, 1, true); // is local
    config.addWorker("0.0.0.0", 18111, 2);
    config.addWorker("0.0.0.0", 18112, 3);

    SearchAggregator ag(mergerProxy, workerProxy);
    ag.setDebug(true);
    ag.setAggregatorConfig(config);

    {
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

    BOOST_CHECK(ag.distributeRequest("", "getKeywordSearchResult", requestGroup, res));

    BOOST_CHECK_EQUAL(res.state, true);
    BOOST_CHECK_EQUAL(res.content, ABC_DOC+CHINESE_DOC+ABC_DOC);
    }

    {
    AddRequest addRequest0(1, 2);
    AddRequest addRequest1(3, 4);
    AddRequest addRequest2(5, 6);

    AddResult addResult;

    RequestGroup<AddRequest, AddResult> addRequestGroup;
    addRequestGroup.addRequest(1, &addRequest0, &addResult);
    addRequestGroup.addRequest(2, &addRequest1, &addResult);
    addRequestGroup.addRequest(3, &addRequest2, &addResult);

    BOOST_CHECK(ag.distributeRequest("", "add", addRequestGroup, addResult));
    BOOST_CHECK_EQUAL(addResult.sum, 1+2+3+4+5+6);
    }
}

BOOST_AUTO_TEST_SUITE_END()
