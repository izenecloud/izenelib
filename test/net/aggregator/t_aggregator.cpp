#include <net/aggregator/JobInfo.h>
#include <net/aggregator/JobAggregator.h>
#include <net/aggregator/AggregatorConfig.h>
#include "data_type.h"

#include <sstream>

using namespace net::aggregator;


class SearchAggregator : public JobAggregator<SearchAggregator>
{
public:

    void join_impl(DataResult& res, const std::vector<std::pair<workerid_t, DataResult> >& resList)
    {
        std::stringstream ss;
        for (size_t i = 0; i < resList.size(); i++)
        {

            ss <<"(worker"<<resList[i].first<< ": "<<resList[i].second.s<<")";
        }

        res.s = ss.str();
    }

    void join_impl(int& res, const std::vector<std::pair<workerid_t, int> >& resList)
    {
        res = 0;
        for (size_t i = 0; i < resList.size(); i++)
        {
           res += resList[i].second;
        }
    }
};

int main( int argc, char * argv[])
{
    AggregatorConfig config;
    config.addWorker("0.0.0.0", 18111);
    config.addWorker("0.0.0.0", 18112);
    config.addWorker("0.0.0.0", 18113);

    SearchAggregator ag;
    ag.setWorkerListConfig(config);

    /// asynchronous requests
    WorkerFutureHolder futureHolder;
    Data req;
    ag.sendRequest<Data>(futureHolder, "getKeywordSearchResult", req);

    WorkerFutureHolder futureHolder2;
    AddData req2; req2.i = 5; req2.j = 100;
    ag.sendRequest<AddData>(futureHolder2, "add", req2);

    // join results
    DataResult result;
    ag.getResult<DataResult>(futureHolder, result);
    std::cout << "keyword result: "<<result.i << " / " <<result.s<< std::endl;

    int result2 = 0;
    ag.getResult<int>(futureHolder2, result2);
    std::cout << "add result: "<<result2<< std::endl;

    /// synchronous
    req2.i = 1;
    result2 = 0;
    std::vector<workerid_t> workeridList;
    workeridList.push_back(2);
    ag.sendRequest<AddData, int>("add", req2, result2, workeridList);
    std::cout << "syn add result: "<<result2<< std::endl;
}

