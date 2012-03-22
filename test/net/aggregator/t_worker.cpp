#include <iostream>
#include <boost/shared_ptr.hpp>

#include "worker_service.h"
#include "TestWorkerController.h"
#include <net/aggregator/WorkerServer.h>

using namespace net::aggregator;


int main( int argc, char * argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: t_worker <host> <port>" << std::endl;
        return 0;
    }

    try
    {
        std::string host = argv[1];
        uint16_t port = atoi(argv[2]);
        std::cout << "[starting job woker..] " << host << ":" << port << std::endl;

        boost::shared_ptr<SearchService> searchService(new SearchService());

        WorkerRouter router;
        TestWorkerController(searchService).addWorkerHandler(router);

        WorkerServer worker(router, host, port);
        worker.start();
        worker.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
        exit(0);
    }

    return 0;
}
