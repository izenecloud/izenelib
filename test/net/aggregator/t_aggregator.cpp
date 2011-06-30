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
};

int main( int argc, char * argv[])
{
    AggregatorConfig config;
    config.addWorker("0.0.0.0", 18111);
    config.addWorker("0.0.0.0", 18112);
    config.addWorker("0.0.0.0", 18113);

    SearchAggregator ag;
    ag.setWorkerListConfig(config);

    // distributed works
    WorkerFutureHolder futureHolder;
    Data req;
    ag.sendRequest<Data>(futureHolder, "getKeywordSearchResult", req);
    DataResult result;
    ag.getResult<DataResult>(futureHolder, result);

    std::cout << "result: "<<result.i << " / " <<result.s<< std::endl;
}

