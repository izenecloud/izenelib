#include <iostream>
#include <net/aggregator/JobWorker.h>

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

    JobWorker worker(host, port);
    worker.Start();

    return 0;
}
