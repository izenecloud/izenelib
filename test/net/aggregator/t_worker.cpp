#include <iostream>
#include <boost/shared_ptr.hpp>

#include "SearchWorker.h"
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

        boost::shared_ptr<SearchWorker> searchWorker(new SearchWorker);

        WorkerRouter router;
        TestWorkerController(searchWorker).addWorkerHandler(router);

        WorkerServer server(router, host, port);
        server.start();
        server.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
        exit(0);
    }

    return 0;
}
