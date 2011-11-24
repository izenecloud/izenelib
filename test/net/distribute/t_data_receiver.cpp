#include <net/distribute/DataReceiver.h>

#include <iostream>
#include <string.h>

using namespace net::distribute;

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
    {
        // disable auto test
        if (strcasecmp(argv[i], "--build_info") == 0)
            return 0;
    }

    unsigned int port = 18121;

    try {
        DataReceiver recv(port);
        recv.start();
    }
    catch (std::exception& e) {
        std::cout<<e.what()<<std::endl;
    }

    return 0;
}

