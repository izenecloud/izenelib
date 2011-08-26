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


class SearchAggregator : public JobAggregator<SearchAggregator>
{
public:

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

public:


private:

};


BOOST_AUTO_TEST_SUITE( t_aggregator )

//BOOST_AUTO_TEST_CASE( start_workers )
//{
//    std::cout << "Start Workers" << std::endl;
//
//    std::string host = "0.0.0.0";
//    uint16_t port = 18151;
//    boost::shared_ptr<SearchService> searchService(new SearchService());
//    WorkerServer worker(host, port, searchService);
//    thread t(lambda::bind(&WorkerServer::start, var(worker)));
//
//    t.join();
//}

BOOST_AUTO_TEST_CASE( aggregator_remote_all )
{
    std::cout << "--- Aggregator Remote All" << std::endl;

    AggregatorConfig config;
    config.enableLocalWorker_ = false;
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

BOOST_AUTO_TEST_CASE( aggregator_local_remote )
{
    std::cout << "--- Aggregator Local and Remote" << std::endl;

    AggregatorConfig config;
    config.enableLocalWorker_ = true;
    config.addWorker("0.0.0.0", 18111);


    SearchAggregator ag;
    boost::shared_ptr<MockWorkerCaller> localWorkerCaller(new MockWorkerCaller);
    ag.debug_ = true;
    ag.setAggregatorConfig(config);
    ag.setLocalWorkerCaller(localWorkerCaller);

    SearchRequest req;
    req.keyword = TERM_ABC;
    SearchResult res;
    ag.distributeRequest("", "getKeywordSearchResult", req, res);

    BOOST_CHECK_EQUAL(res.state, true);
    BOOST_CHECK_EQUAL(res.content, (ABC_DOC+ABC_DOC));
}

BOOST_AUTO_TEST_SUITE_END()
