#include <net/aggregator/JobAggregator.h>
#include <net/aggregator/AggregatorConfig.h>

using namespace net::aggregator;


int main( int argc, char * argv[])
{
    AggregatorConfig config;
    config.addWorker("0.0.0.0", 18111);
    config.addWorker("0.0.0.0", 18112);

    JobAggregator ja;
    ja.setWorkerListConfig(config);

    ja.request("add");
}

