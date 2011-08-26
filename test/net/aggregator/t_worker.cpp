#include <iostream>
#include <net/aggregator/JobInfo.h>
#include <net/aggregator/JobWorker.h>
#include <net/aggregator/WorkerHandler.h>

#include <boost/shared_ptr.hpp>

#include "worker_service.h"
#include "worker_server.h"

using namespace net::aggregator;


int main( int argc, char * argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: t_worker <host> <port>" << std::endl;
        return -1;
    }

    std::string host = argv[1];
    uint16_t port = atoi(argv[2]);
    std::cout <<"[starting job woker..] "<< host << ":" << port <<std::endl;


    boost::shared_ptr<SearchService> searchService(new SearchService());

    WorkerServer worker(host, port, searchService);
    worker.debug_ = true;
    worker.start();
    worker.join();
    return 0;
}
