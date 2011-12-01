#include <net/distribute/DataReceiver.h>

#include <iostream>
#include <string.h>

using namespace net::distribute;

int main(int argc, char** argv)
{
    std::string baseDir = ".";
    unsigned int port = 18121;

    char optchar;
    bool help = false;
    while ((optchar = getopt(argc, argv, "h:p:d:")) != -1)
    {
        switch (optchar) {
            case 'h':
                help = true;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
                baseDir = optarg;
                break;
            default:
                std::cout << "Unrecognized flag " << optchar << std::endl;
                help = true;
                break;
        }

        if (help)
            break;
    }

    if (help)
    {
        std::cout<<"Usage: "<<argv[0]<<" -d <data-base-dir> -p port "<<std::endl;
        return 0;
    }

    try {
        DataReceiver recv(port, baseDir);
        recv.start();
    }
    catch (std::exception& e) {
        std::cout<<e.what()<<std::endl;
    }

    return 0;
}

