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

    Data req;
    //Data ret;
    DataResult ret;
    ag.sendRequest<Data, DataResult>("getKeywordSearchResult", req, ret);

    std::cout << "result: "<<ret.i << " / " <<ret.s<< std::endl;
}

